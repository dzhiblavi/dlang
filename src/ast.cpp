#include "ast.h"

#include <utility>

namespace {
std::string block(std::vector<sh_st_p> const& nodes, int depth = 1) {
    std::string ret = "{\n";
    std::string tab = depth == 1 ? "    " : "        ";
    for (auto const& n : nodes) {
        ret += tab + n->to_string() + "\n";
    }
    ret += std::string((depth == 2 ? "    " : "")) + "}";
    return ret;
}

std::string func_call(std::vector<sh_value_p> const& values) {
    std::string ret = "(";
    for (size_t i = 0; i < values.size(); ++i) {
        ret += values[i]->to_string();
        if (i != values.size() - 1) {
            ret += " ";
        }
    }
    return ret + ")";
}

std::string op_to_string(arithm::OP op) {
    switch (op) {
        case arithm::PLUS:
            return "+";
        case arithm::MINUS:
            return "-";
    }
}
}

context::context(context* ctx)
    : parent(ctx) {}

void context::add_var(std::string const& name, sh_var_p variable) {
    variables[name] = std::move(variable);
}

void context::add_fun(std::string const& name, sh_func_p function) {
    functions[name] = std::move(function);
}

sh_var_p context::is_var(std::string const& name) {
    auto i = variables.find(name);
    if (i != variables.end()) {
        return i->second;
    }
    return parent ? parent->is_var(name) : nullptr;
}

sh_func_p context::is_fun(std::string const& name) {
    auto f = functions.find(name);
    if (f != functions.end()) {
        return f->second;
    }
    return parent ? parent->is_fun(name) : nullptr;
}

void context::load_context(state &ctx) {
    for (auto& p : variables) {
        ctx.var_alloc(p.second->quialified_name());
    }
}

void context::free_context(state &ctx) {
    for (auto it = variables.rbegin(); it != variables.rend(); ++it) {
        ctx.var_free(it->second->quialified_name());
    }
}

int value::alloc_sz() {
    return 1;
}


var::var(std::string name)
    : name(std::move(name)) {}

std::string var::quialified_name() {
    return name;
}

std::string var::set(int cell, int size, state& ctx) {
    std::string code;
    int addr = ctx.var_addr(quialified_name());
    code += ctx.mov(cell, addr);
    return code;
}

std::string var::set_to(int cell, state& ctx) {
    std::string code;
    int tmp = ctx.stalloc();
    int addr = ctx.var_addr(quialified_name());
    code += ctx.copy(addr, cell, tmp);
    ctx.stdealloc(tmp);
    return code;
}

std::string var::to_string() const {
    return name;
}


//arr::arr(sh_value_p index)
//        : index(std::move(index)) {}

arr::arr(sh_num_p index)
        : index(std::move(index)) {}

int arr::address(state& ctx) {
    switch (t) {
        case GLOB:
            return USER;
        case PRM:
            return ctx.parmcell();
        case USR:
            return ctx.var_addr(usr_base->quialified_name());
    }
}

std::string arr::set(int cell, int size, state& ctx) {
    std::string code;
    int tmp = ctx.stalloc();
    int i = 0;
    while (size--) {
        code += ctx.copy(cell + i, address(ctx) + index->value + i, tmp);
        ++i;
    }
    ctx.stdealloc(tmp);
    return code;
}

std::string arr::set_to(int cell, state& ctx) {
    std::string code;
    int tmp = ctx.stalloc();
    code += ctx.copy(address(ctx) + index->value, cell, tmp);
    ctx.stdealloc(tmp);
    return code;
}

std::string arr::to_string() const {
    std::string name;
    switch (t) {
        case GLOB:
            name = "a";
            break;
        case PRM:
            name = "p";
            break;
        default:
            name = usr_base->to_string();
    }
    return name + "[" + index->to_string() + "]";
}


num::num(int value)
    : value(value) {}

std::string num::set_to(int cell, state& ctx) {
    return ctx.set(cell, value);
}

std::string num::to_string() const {
    return std::to_string(value);
}


str::str(std::string s)
    : value(std::move(s)) {}

int str::alloc_sz() {
    return value.size();
}

std::string str::set_to(int cell, state& ctx) {
    std::string code;
    for (int i = 0; i < (int)value.size(); ++i) {
        code += ctx.set(cell + i, value[i]);
    }
    return code;
}

std::string str::to_string() const {
    return "\"" + value + "\"";
}


int arry::alloc_sz() {
    int ret = 0;
    for (auto const& v : values) {
        ret += v->alloc_sz();
    }
    return ret;
}

std::string arry::set_to(int cell, state& ctx) {
    std::string code;
    for (int i = 0; i < (int)values.size(); ++i) {
        code += values[i]->set_to(cell + i, ctx);
    }
    return code;
}

std::string arry::to_string() const {
    std::string ret = "{ ";
    for (size_t i = 0; i < values.size(); ++i) {
        ret += values[i]->to_string();
        if (i != values.size() - 1) {
            ret += " ";
        }
    }
    return ret + " }";
}


arithm::arithm(sh_value_p l, sh_value_p r, OP op)
    : l(std::move(l)), r(std::move(r)), op(op) {}

std::string arithm::set_to(int cell, state& ctx) {
    std::string code;
    int tl = ctx.stalloc();
    int tr = ctx.stalloc();
    code += l->set_to(tl, ctx);
    code += r->set_to(tr, ctx);

    switch (op) {
        case PLUS:
            code += ctx.add(tr, tl);
            break;
        case MINUS:
            code += ctx.sub(tr, tl);
            break;
    }

    code += ctx.mov(tl, cell);
    ctx.stdealloc(tr);
    ctx.stdealloc(tl);
    return code;
}

std::string arithm::to_string() const {
    return l->to_string() + " " + op_to_string(op) + " " + r->to_string();
}


while_st::while_st(context* ctx)
    : context(ctx) {}

std::string while_st::encode(state& ctx) {
    std::string code;
    int cond = ctx.stalloc();
    load_context(ctx);

    code += condition->set_to(cond, ctx);
    code += ctx.sh(cond);
    code += "[";
    for (auto const& s : sts) {
        code += s->encode(ctx);
    }
    code += condition->set_to(cond, ctx);
    code += ctx.sh(cond);
    code += "]";

    free_context(ctx);
    ctx.stdealloc(cond);
    return code;
}

std::string while_st::to_string() const {
    return "while " + condition->to_string() + " " + block(sts, 2);
}


if_st::if_st(context* ctx)
    : context(ctx) {}

std::string if_st::encode(state& ctx) {
    if (neg.empty()) {
        std::string code;
        int cond = ctx.stalloc();
        load_context(ctx);

        code += condition->set_to(cond, ctx);
        code += ctx.sh(cond);
        code += "[";
        for (auto const& s : pos) {
            code += s->encode(ctx);
        }
        code += "[-]]";

        free_context(ctx);
        ctx.stdealloc(cond);
        return code;
    } else {
        std::string code;

        int cond = ctx.stalloc();
        int tmp = ctx.stalloc();
        load_context(ctx);

        code += condition->set_to(cond, ctx);
        code += ctx.set(tmp, 1);
        code += ctx.sh(cond);
        code += "[";
        for (auto const& s : pos) {
            code += s->encode(ctx);
        }
        code += ctx.set(tmp, 0);
        code += ctx.sh(cond);
        code += "[-]]";

        code += ctx.sh(tmp);
        code += "[";
        for (auto const& s : neg) {
            code += s->encode(ctx);
        }
        code += ctx.sh(tmp);
        code += "[-]]";

        free_context(ctx);
        ctx.stdealloc(tmp);
        ctx.stdealloc(cond);
        return code;
    }
}

std::string if_st::to_string() const {
    return "if " + condition->to_string() + " " + block(pos, 2) + " else " + block(neg, 2);
}


set_st::set_st(sh_lvalue_p lh, sh_value_p rh)
    : lh(std::move(lh)), rh(std::move(rh)) {}

std::string set_st::encode(state& ctx) {
    std::string code;
    int tmp = ctx.stalloc(rh->alloc_sz());
    code += rh->set_to(tmp, ctx);
    code += lh->set(tmp, rh->alloc_sz(), ctx);
    ctx.stdealloc(tmp, rh->alloc_sz());
    return code;
}

std::string set_st::to_string() const {
    return lh->to_string() + " = " + rh->to_string();
}


func::func(context* ctx, std::string name)
    : context(ctx), name(std::move(name)) {}

std::string func::to_string() const {
    return name + " " + block(sts, 1);
}


func_st::func_st(sh_func_p f)
    : f(std::move(f)) {}

std::string func_st::set_to(int cell, state& ctx) {
    std::string code;

    int old_parm = ctx.parmcell();
    ctx.parmalloc(parms.size());

    int i = 0;
    for (auto const& s : parms) {
        int palloc = ctx.parmcell();
        ctx.force_setparm(old_parm);
        code += s->set_to(palloc + i++, ctx);
        ctx.force_rmparm(old_parm);
    }

    ctx.retalloc();
    f->load_context(ctx);

    for (auto const& s : f->sts) {
        code += s->encode(ctx);
    }
    code += ctx.mov(ctx.retcell(), cell);

    f->free_context(ctx);
    ctx.retdealloc(ctx.retcell());
    ctx.parmdealloc(ctx.parmcell(), parms.size());
    return code;
}

std::string func_st::encode(state& ctx) {
    std::string code;
    if (f->name == "putNATIVE") {
        code += ctx.sh(BASE_CELL);
        code += ".";
    } else if (f->name == "readNATIVE") {
        code += ctx.sh(BASE_CELL);
        code += ",";
    } else {
        int old_parm = ctx.parmcell();
        ctx.parmalloc(parms.size());

        int i = 0;
        for (auto const& s : parms) {
            int palloc = ctx.parmcell();
            ctx.force_setparm(old_parm);
            code += s->set_to(palloc + i++, ctx);
            ctx.force_rmparm(old_parm);
        }

        ctx.retalloc();
        f->load_context(ctx);

        for (auto const& s : f->sts) {
            code += s->encode(ctx);
        }

        f->free_context(ctx);
        ctx.retdealloc(ctx.retcell());
        ctx.parmdealloc(ctx.parmcell(), parms.size());
        return code;
    }
    return code;
}

std::string func_st::to_string() const {
    return f->name + func_call(parms);
}


ret_st::ret_st(sh_value_p v)
    : v(std::move(v)) {}

std::string ret_st::encode(state &ctx) {
    std::string code;
    code += v->set_to(ctx.retcell(), ctx);
    return code;
}

std::string ret_st::to_string() const {
    return "return " + v->to_string();
}


program::program()
    : context(nullptr) {
    std::vector<std::string> native = {
            "putNATIVE",
            "readNATIVE"
    };

    for (std::string const& s : native) {
        sh_func_p f = std::make_shared<func>(this, s);
        this->add_fun(s, f);
        funcs.push_back(f);
    }
}

std::string program::encode(state& ctx) {
    for (auto const& f : funcs) {
        if (f->name == "main") {
            ctx.force_setparm(0);
            f->load_context(ctx);
            std::string code;
            for (auto const& s : f->sts) {
                code += s->encode(ctx);
            }
            f->free_context(ctx);
            ctx.force_rmparm(0);
            return code;
        }
    }

    throw std::runtime_error("no main() function found");
}

std::string program::to_string() const {
    std::string ret;
    for (auto const& f : funcs) {
        ret += f->to_string() + "\n";
    }
    return ret;
}

