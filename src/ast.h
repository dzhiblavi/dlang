#ifndef DLANG_AST_H
#define DLANG_AST_H

#include <string>
#include <map>
#include <vector>

struct ast_node {
    virtual std::string to_string() const = 0;
};

struct value {
    virtual std::string to_string() const = 0;
};

typedef std::shared_ptr<value> sh_v_p;

struct func : ast_node {
    std::string name;
    int nargs;
    sh_v_p value;
    std::vector<std::string> argnames;

public:
    func(std::string const& name, int nargs);

    std::string to_string() const override;
};

typedef std::shared_ptr<func> sh_f_p;

struct program : ast_node {
    std::map<std::string, sh_f_p> funcs;

public:
    program(std::map<std::string, sh_f_p> const& funcs);

    std::string to_string() const override;
};

typedef std::shared_ptr<program> sh_p_p;

struct str : value {
    std::string val;

public:
    str(std::string const& val);

    std::string to_string() const override;
};

struct chv : value {
    char c;

public:
    chv(char c);

    std::string to_string() const override;
};

struct call : value {
    sh_f_p f;
    std::vector<sh_v_p> args;

public:
    call(sh_f_p const& f);

    std::string to_string() const override;
};

struct arg : value {
    std::string name;
    int index;

public:
    arg(std::string const& name, int index);

    std::string to_string() const override;
};


#endif //DLANG_AST_H
