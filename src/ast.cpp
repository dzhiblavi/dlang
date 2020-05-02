#include "ast.h"

func::func(std::string const &name, int nargs)
    : name(name), nargs(nargs) {}

std::string func::to_string() const {
    std::string s = name + " : " + std::to_string(nargs);
    if (value)
        s += "\n" + name + " = " + value->to_string();
    return s;
}


program::program(std::map<std::string, sh_f_p> const &funcs)
    : funcs(funcs) {}

std::string program::to_string() const {
    std::string s;
    for (auto& f : funcs) {
        s += f.second->to_string() + "\n\n";
    }
    return s;
}


str::str(std::string const& val)
    : val(val) {}

std::string str::to_string() const {
    return std::string("\"") + val + "\"";
}


chv::chv(char c)
    : c(c) {}

std::string chv::to_string() const {
    return std::string("'") + c + "'";
}


call::call(sh_f_p const &f)
    : f(f) {}

std::string call::to_string() const {
    std::string s = f->name;
    for (auto& arg : args) {
        s += " " + arg->to_string();
    }
    return s;
}


arg::arg(std::string const& name, int index)
    : name(name), index(index) {}

std::string arg::to_string() const {
    return name;
}
