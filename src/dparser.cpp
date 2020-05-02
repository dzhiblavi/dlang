#include "dparser.h"

namespace {
[[noreturn]] void fail(std::string const& why) {
    throw std::runtime_error("parse failed: " + why);
}
}

dparser::dparser(dlexer &l)
    : l(l) {}

void dparser::assert_next(TOKEN t) {
    if (l.lookup().first != t)
        fail("unexpected token: " + l.lookup().second);
}

int dparser::parse_int() {
    assert_next(NUM);
    return atoi(l.next_token().second.c_str());
}

sh_f_p dparser::parse_declaration() {
    assert_next(IDENT);
    dlexer::token t = l.next_token();
    dlexer::token tt = l.next_token();

    if (tt.first != COLON) {
        l.putback(tt);
        l.putback(t);
        return nullptr;
    }

    std::string const& name = t.second;
    int nargs = parse_int();

    return std::make_shared<func>(name, nargs);
}

std::map<std::string, int> dparser::parse_args_declaration(int nargs) {
    std::map<std::string, int> ret;
    for (int i = 0; i < nargs; ++i) {
        assert_next(IDENT);
        ret[l.next_token().second] = i;
    }
    return ret;
}

std::unique_ptr<str> dparser::parse_str_value() {
    assert_next(QT);
    l.next_token();
    dlexer::token t = l.next_token();
    std::string v;
    while (t.first != QT && t.first != END) {
        v += t.second;
        t = l.next_token();
    }
    if (t.first == END)
        fail("no pair \" found");
    l.putback(t);
    assert_next(QT);
    l.next_token();
    return std::make_unique<str>(v);
}

std::unique_ptr<call> dparser::parse_call_value(std::map<std::string, int>& args) {
    assert_next(IDENT);
    std::string const& fname = l.next_token().second;

    if (fs.find(fname) == fs.end())
        fail("unknown identifier: " + fname);
    sh_f_p f = fs[fname];

    std::unique_ptr<call> c = std::make_unique<call>(f);
    for (int i = 0; i < f->nargs; ++i) {
        c->args.push_back(parse_value(args));
    }
    return c;
}

std::unique_ptr<arg> dparser::parse_arg_value(std::map<std::string, int>& args) {
    assert_next(IDENT);
    std::string const& name = l.next_token().second;

    if (args.find(name) == args.end())
        fail("unknown identifier: " + name);

    return std::make_unique<arg>(name, args[name]);
}

std::unique_ptr<chv> dparser::parse_chv_value() {
    assert_next(QS);
    l.next_token();
    dlexer::token t = l.next_token();
    char c;
    if (t.second.size() != 1)
        fail("invalid character: " + t.second);
    c = t.second[0];
    assert_next(QS);
    l.next_token();
    return std::make_unique<chv>(c);
}

sh_v_p dparser::parse_value(std::map<std::string, int>& args) {
    dlexer::token t = l.lookup();

    switch (t.first) {
        case QT:
            return sh_v_p(parse_str_value().release());
        case QS:
            return sh_v_p(parse_chv_value().release());
        case IDENT:
            if (args.find(t.second) != args.end())
                return sh_v_p(parse_arg_value(args).release());
            else
                return sh_v_p(parse_call_value(args).release());
        default:
            fail("value not supported: " + t.second);
    }
}

void dparser::parse_definition() {
    assert_next(IDENT);
    dlexer::token t = l.next_token();
    std::string const& name = t.second;

    if (fs.find(name) == fs.end())
        fail("unknown identifier: " + name);
    sh_f_p f = fs[name];

    std::map<std::string, int> args = parse_args_declaration(f->nargs);
    f->argnames.resize(f->nargs);
    for (auto const& s : args) {
        f->argnames[s.second] = s.first;
    }

    if (l.next_token().first != EQ)
        fail("'=' expected; found: " + l.lookup().second);

    f->value = parse_value(args);
}

sh_p_p dparser::parse() {
    while (l.lookup().first != END) {
        sh_f_p f = parse_declaration();
        if (!f)
            break;
        if (fs.find(f->name) != fs.end())
            fail("function redeclaration");
        fs[f->name] = f;
    }
    while (l.lookup().first != END) {
        parse_definition();
    }
    return std::make_shared<program>(fs);
}

