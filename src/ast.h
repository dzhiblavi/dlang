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
    virtual void compile(std::string& s, std::string const& ret) = 0;
    virtual std::string node_name() = 0;
};

typedef std::shared_ptr<value> sh_v_p;

struct func : ast_node {
    std::string name;
    int nargs;
    sh_v_p value;
    std::vector<std::string> argnames;

public:
    func(std::string const& name, int nargs);

    std::string func_name(std::string const& uid);

    void compile(std::string const& uid, std::string const& ret, std::string& s);

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

    std::string ith(int i);

public:
    str(std::string const& val);

    void compile(std::string& s, std::string const& ret) override;

    std::string node_name() override;

    std::string to_string() const override;
};

struct call : value {
    sh_f_p f;
    std::vector<sh_v_p> args;

    std::string argi(int i);

public:
    call(sh_f_p const& f);

    void compile(std::string& s, std::string const& ret) override;

    std::string node_name() override;

    std::string to_string() const override;
};

struct arg : value {
    std::string name;
    int depth;
    int index;

public:
    arg(std::string const& name, int index, int depth);

    void compile(std::string& s, std::string const& ret) override;

    std::string node_name() override;

    std::string to_string() const override;
};

struct ife : value {
    sh_v_p cond;
    sh_v_p pos;
    sh_v_p neg;

public:
    ife(sh_v_p const& cond, sh_v_p const& pos, sh_v_p const& neg);

    void compile(std::string& s, std::string const& ret) override;

    std::string node_name() override;

    std::string to_string() const override;
};


#endif //DLANG_AST_H
