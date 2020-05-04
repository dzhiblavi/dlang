#include "ast.h"
#include "dtransmt.h"

namespace {
std::map<void*, std::string> uids_;
int maxid = 0;

std::string nextuid() {
    return std::to_string(maxid++);
}

std::string uid(void* s) {
    if (uids_.find(s) == uids_.end()) {
        uids_[s] = nextuid();
    }
    return uids_[s];
}
}

func::func(std::string const &name, int nargs)
    : name(name), nargs(nargs) {}


std::string user_func::func_name(std::string const& uid) {
    return "func_" + name + uid;
}

user_func::user_func(std::string const &name, int nargs)
    : func(name, nargs) {}

std::string user_func::to_string() const {
    std::string s = name + " : " + std::to_string(nargs);
    if (value) {
        s += "\n" + name + " ";
        for (auto &an : argnames)
            s += an + " ";
        s += "= " + value->to_string();
    }
    return s;
}

void user_func::compile(std::string const& uid, std::string const& ret, std::string& s) {
    if (!value)
        throw std::runtime_error("no function definition: " + name);
    s += func_name(uid) + " $ -> " + value->node_name() + " $ <\n\n";
    value->compile(s, ret);
}


native_func::native_func(std::string const &name, int nargs)
    : func(name, nargs) {}

void native_func::compile(const std::string &uid, const std::string &ret, std::string &s) {
    assert(!tmplt.empty());
    std::stringstream ss(tmplt);
    dtransmt::load_native_template(s, ss, uid, ret);
}

std::string native_func::func_name(std::string const& uid) {
    return name + uid;
}

std::string native_func::to_string() const {
    return name + " : " + std::to_string(nargs);
}


program::program(std::map<std::string, sh_uf_p> const &funcs)
    : funcs(funcs) {}

std::string program::to_string() const {
    std::string s;
    for (auto& f : funcs)
        s += f.second->to_string() + "\n\n";
    return s;
}


str::str(std::string const& val)
    : val(val) {}

std::string str::ith(int i) {
    return "__str_" + uid(this) + "_" + std::to_string(i);
}

std::string str::node_name() {
    return ith(0);
}

void str::compile(std::string &s, std::string const& ret) {
    if (val.empty()) {
        s += ith(0) + " _ -> " + ret + " _ ^\n";
    } else if (val.size() > 1) {
        s += ith(0) + " _ -> " + ith(1) + " " + val.back() + " <\n";
        for (int i = val.size() - 2, j = 1; i >= 1; --i, ++j)
            s += ith(j) + " _ -> " + ith(j + 1) + " " + val[i] + " <\n";
        s += ith(val.size() - 1) + " _ -> " + ret + " " + val[0] + " ^\n";
    } else {
        s += ith(val.size() - 1) + " _ -> " + ret + " " + val[0] + " ^\n";
    }
}

std::string str::to_string() const {
    return std::string("\"") + val + "\"";
}


call::call(sh_f_p const &f)
    : f(f) {}

std::string call::node_name() {
    return "__call_" + f->name + "_" + uid(this);
}

std::string call::argi(int i) {
    return "__put_arg_" + std::to_string(i) + "_" + uid(this);
}

void call::compile(std::string &s, const std::string &ret) {
    std::string ud = uid(this);
    if (!args.empty()) {
        s += node_name() + " _ -> " + argi(0) + " $ <\n\n";

        for (int i = 0; i < (int)args.size(); ++i) {
            s += argi(i) + " _ -> " + args[args.size() - 1 - i]->node_name() + " _ ^\n";
        }
        s += "\n";

        for (int i = 0; i < (int)args.size() - 1; ++i) {
            std::string shl1 = "__sh_" + nextuid();
            std::string shl2 = "__sh_" + nextuid();

            args[args.size() - i - 1]->compile(s, shl1);

            s += shl1 + " $ -> " + shl2 + " $ <\n\n";
            s += shl1 + " _ -> " + shl2 + " _ ^\n\n";
            s += shl1 + " # -> " + shl2 + " # <\n\n";
            s += shl1 + " * -> " + shl2 + " * <\n\n";
            s += shl2 + " _ -> " + argi(i + 1) + " # <\n\n";
        }

        std::string fc1 = "__call_stub_" + nextuid();
        std::string fc2 = "__call_stub_" + nextuid();

        args[0]->compile(s, fc1);

        s += fc1 + " _ -> " + fc2 + " _ ^\n\n";
        s += fc1 + " $ -> " + fc2 + " $ <\n\n";
        s += fc1 + " # -> " + fc2 + " # <\n\n";
        s += fc1 + " * -> " + fc2 + " * <\n\n";
        s += fc2 + " _ -> " + f->func_name(ud) + " $ ^\n\n";
        s += "\n";
    } else {
        s += node_name() + " _ -> " + f->func_name(ud) + " $ ^\n\n";
    }

    if (dynamic_cast<native_func*>(f.get())) {
        std::cout << "native: " << f->name << std::endl;
        f->compile(ud, ret, s);
        return;
    }

    std::string return_point = "__return_call_" + nextuid();
    f->compile(ud, return_point, s);

    std::string ur = nextuid();
    std::string native_return = "__native_return_" + ur;
    s += return_point + " * -> " + native_return + " * ^\n\n";
    s += return_point + " $ -> " + native_return + " $ ^\n\n";
    s += return_point + " _ -> " + native_return + " _ ^\n\n";

    dtransmt::compile_native("__native_return_", s, ur, ret);
}

std::string call::to_string() const {
    std::string s = f->name;
    for (auto& arg : args) {
        s += " " + arg->to_string();
    }
    return s;
}


arg::arg(std::string const& name, int index, int depth)
    : name(name), index(index), depth(depth) {}

std::string arg::node_name() {
    return "__arg_" + uid(this);
}

std::string skip_calls(int depth, std::string& s, std::string const& ret) {
    depth++;
    std::vector<std::string> rs;

    for (int i = 0; i < depth; ++i) {
        rs.push_back("__skip_call" + std::to_string(i) + nextuid());
    }

    for (int i = 0; i < depth; ++i) {
        std::string const& node = rs[i];

        s += node + " # -> " + node + " # >\n";
        s += node + " _ -> " + node + " _ >\n";
        s += node + " * -> " + node + " * >\n";

        if (i + 1 < depth) {
            s += node + " $ -> " + rs[i + 1] + " $ >\n";
        } else {
            s += node + " $ -> " + ret + " $ >\n";
        }
    }

    return rs.front();
}

std::string skip_args(int index, std::string& s, std::string const& ret) {
    if (index == 0) {
        return ret;
    }

    std::vector<std::string> rs;

    for (int i = 0; i < index; ++i) {
        rs.push_back("__skip_args" + std::to_string(i) + nextuid());
    }

    for (int i = 0; i < index; ++i) {
        std::string const& node = rs[i];

        s += node + " * -> " + node + " * >\n";

        if (i + 1 < index) {
            s += node + " # -> " + rs[i + 1] + " # >\n";
        } else {
            s += node + " # -> " + ret + " # >\n";
        }
    }

    return rs.front();
}

void arg::compile(std::string &s, const std::string &ret) {
    std::string ud = nextuid();
    std::string arg_copy = "__arg_copy_" + ud;

    dtransmt::compile_native("__arg_copy_", s, ud, ret);

    std::string sk_arg_entry = skip_args(index, s, arg_copy);
    std::string sk_call_entry = skip_calls(depth, s, sk_arg_entry);

    s += node_name() + " _ -> " + sk_call_entry + " _ ^\n\n";
}

std::string arg::to_string() const {
    return name + std::to_string(depth);
}


ife::ife(sh_v_p const &cond, sh_v_p const &pos, sh_v_p const &neg)
    : cond(cond), pos(pos), neg(neg) {}

std::string ife::node_name() {
    return "__ife_" + uid(this);
}

void ife::compile(std::string &s, const std::string &ret) {
    std::string cond_ret = "__cond_ret" + nextuid();
    std::string ud = nextuid();
    std::string clear_node = "__clear_" + ud;
    std::string clear_return = "__clear_return_" + nextuid();

    s += node_name() + " _ -> " + cond->node_name() + " _ ^\n\n";
    s += cond_ret + " _ -> " + neg->node_name() + " _ ^\n\n";
    s += cond_ret + " * -> " + clear_node + " * ^\n\n";
    s += clear_return + " _ -> " + pos->node_name() + " _ ^\n\n";

    cond->compile(s, cond_ret);
    pos->compile(s, ret);
    neg->compile(s, ret);

    dtransmt::compile_native("__clear_", s, ud, ret);
}

std::string ife::to_string() const {
    return "if " + cond->to_string() + " then " + pos->to_string() + " else " + neg->to_string();
}