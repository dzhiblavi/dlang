#include "dparser.h"

#include <utility>


namespace {
[[noreturn]] void fail(std::string const &why) {
    throw std::runtime_error(why);
}
}

void dparser::assert_next(TOKEN tkn) {
    auto t = l.next_token();
    if (tkn != t.first) {
        fail("parse failed: " + t.second + " was not expected");
    }
}

dparser::dparser(dlexer lexer)
    : l(std::move(lexer)) {}

std::string dparser::parse_identifier() {
    dlexer::token t = l.next_token();
    if (t.first != IDENT) {
        fail("identifier expected. found '" + t.second + "'");
    }
    return t.second;
}

sh_var_p dparser::parse_variable(context* ctx, bool defined) {
    std::string name = parse_identifier();
    sh_var_p lv = ctx->is_var(name);
    if (nullptr != lv) {
        return lv;
    }
    if (defined) {
        fail("unknown identifier: '" + name + "'");
    }
    sh_var_p v = std::make_shared<var>(name);
    ctx->add_var(name, v);
    return v;
}

sh_lvalue_p dparser::parse_lvalue(context *ctx, bool defined) {
    dlexer::token t = l.next_token();

    if (l.lookup().first == SOP) {
        l.putback(t);
        return parse_array_subscript(ctx);
    } else {
        l.putback(t);
        return parse_variable(ctx, defined);
    }
}

sh_value_p dparser::parse_string() {
    assert_next(QT);
    std::string value;
    while (l.lookup().first != QT && l.lookup().first != END) {
        value += l.next_token().second;
    }
    assert_next(QT);
    return sh_value_p(new str(value));
}

sh_value_p dparser::parse_array(context* ctx) {
    assert_next(COP);
    std::unique_ptr<arry> ar = std::make_unique<arry>();
    while (l.lookup().first != CCL && l.lookup().first != END) {
        ar->values.push_back(parse_expr(ctx));
    }
    assert_next(CCL);
    return sh_value_p(ar.release());
}

sh_value_p dparser::parse_char() {
    assert_next(QS);
    dlexer::token t = l.next_token();
    if (t.second.size() != 1) {
        fail("character expected. found: '" + t.second + "'");
    }
    assert_next(QS);
    return sh_value_p(new num((int)t.second[0]));
}

std::unique_ptr<num> dparser::parse_num() {
    dlexer::token t = l.next_token();
    if (t.first != NUM) {
        fail("integer expected");
    }
    return std::make_unique<num>(atoi(t.second.c_str()));
}

std::unique_ptr<arr> dparser::parse_array_subscript(context* ctx) {
    std::string name = parse_identifier();
    assert_next(SOP);
    sh_num_p index = parse_num();
    std::unique_ptr<arr> ret = std::make_unique<arr>(index);
    assert_next(SCL);

    if (name.size() > 1 || (name[0] != 'a' && name[0] != 'p')) {
        ret->t = arr::USR;
        ret->usr_base = ctx->is_var(name);
        if (ret->usr_base == nullptr) {
            fail("unknown identifier: '" + name + "'");
        }
        return ret;
    }

    switch (name[0]) {
        case 'a':
            ret->t = arr::GLOB;
            break;
        case 'p':
            ret->t = arr::PRM;
            break;
    }
    return ret;
}

sh_value_p dparser::parse_expr(context* ctx) {
    sh_value_p val = parse_value(ctx);

    arithm::OP op;
    switch (l.lookup().first) {
        case PLUS:
            op = arithm::PLUS;
            l.next_token();
            break;
        case MINUS:
            op = arithm::MINUS;
            l.next_token();
            break;
        default:
            return val;
    }

    sh_value_p s_val = parse_expr(ctx);
    return sh_value_p(new arithm(val, s_val, op));
}

sh_value_p dparser::parse_value(context *ctx) {
    dlexer::token t = l.lookup();

    switch (t.first) {
        case IDENT:
            if (ctx->is_fun(t.second)) {
                return sh_value_p(parse_fun_st(ctx).release());
            } else {
                return parse_lvalue(ctx, true);
            }
        case NUM:
            return parse_num();
        case QT:
            return parse_string();
        case QS:
            return parse_char();
        case COP:
            return parse_array(ctx);
        default:
            fail("value expected. found: '" + t.second + "'");
    }
}

sh_st_p dparser::parse_while(context* ctx) {
    std::unique_ptr<while_st> w = std::make_unique<while_st>(ctx);
    w->condition = parse_expr(ctx);
    parse_block(w.get(), w->sts);
    return sh_st_p(w.release());
}

sh_st_p dparser::parse_if(context* ctx) {
    std::unique_ptr<if_st> ifst = std::make_unique<if_st>(ctx);
    ifst->condition = parse_expr(ctx);
    parse_block(ifst.get(), ifst->pos);
    if (l.lookup().first == ELSE) {
        l.skip();
        parse_block(ifst.get(), ifst->neg);
    }
    return sh_st_p(ifst.release());
}

void dparser::parse_fun_call(context* ctx, std::vector<sh_value_p>& sts) {
    assert_next(POP);
    sh_value_p v;
    while (l.lookup().first != PCL && nullptr != (v = parse_expr(ctx))) {
        sts.push_back(v);
    }
    assert_next(PCL);
}

std::unique_ptr<func_st> dparser::parse_fun_st(context* ctx) {
    std::string name = parse_identifier();
    std::unique_ptr<func_st> fst = std::make_unique<func_st>(ctx->is_fun(name));
    parse_fun_call(ctx, fst->parms);
    return fst;
}

sh_st_p dparser::parse_set(context* ctx) {
    sh_lvalue_p lh = parse_lvalue(ctx, false);
    assert_next(EQ);
    sh_value_p rh = parse_expr(ctx);

    return sh_st_p(new set_st(lh, rh));
}

sh_st_p dparser::parse_statement(context* ctx) {
    dlexer::token t = l.lookup();
    switch (t.first) {
        case IDENT:
            if (ctx->is_fun(t.second)) {
                return sh_st_p(parse_fun_st(ctx).release());
            } else {
                return parse_set(ctx);
            }
        case WHILE:
            l.skip();
            return parse_while(ctx);
        case IF:
            l.skip();
            return parse_if(ctx);
        case RET:
            l.skip();
            return sh_st_p(new ret_st(parse_expr(ctx)));
        default:
            fail("statement expected. found '" + l.lookup().second + "'");
    }
}

void dparser::parse_block(context *ctx, std::vector<sh_st_p> &sts) {
    assert_next(COP);
    sh_st_p st;
    while (l.lookup().first != CCL && nullptr != (st = parse_statement(ctx))) {
        sts.push_back(st);
    }
    assert_next(CCL);
}

sh_func_p dparser::parse_function(context *ctx) {
    if (l.lookup().first == END) {
        return nullptr;
    }
    std::string name = parse_identifier();
    sh_func_p f = std::make_shared<func>(ctx, name);
    ctx->add_fun(name, f);
    parse_block(f.get(), f->sts);
    return f;
}

sh_prog_p dparser::parse() {
    sh_prog_p p = std::make_shared<program>();
    sh_func_p f;
    while (l.lookup().first != END && nullptr != (f = parse_function(p.get()))) {
        p->funcs.push_back(f);
    }
    return p;
}

