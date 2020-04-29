#ifndef TEST_AST_H
#define TEST_AST_H

#include <string>
#include <map>
#include <vector>
#include <iostream>

#include "trans.h"


struct ast_node {
    virtual ~ast_node() = default;
    [[nodiscard]] virtual std::string to_string() const = 0;
};

struct statement : virtual ast_node {
    virtual std::string encode(state& ctx) = 0;
};

struct value : virtual ast_node {
    virtual int alloc_sz();
    virtual std::string set_to(int cell, state& ctx) = 0;
};

struct lvalue : value {
    virtual std::string set(int cell, int size, state& ctx) = 0;
};

typedef std::shared_ptr<value> sh_value_p;
typedef std::shared_ptr<lvalue> sh_lvalue_p;
typedef std::shared_ptr<statement> sh_st_p;

struct var : lvalue {
    std::string name;

public:
    explicit var(std::string name);

    std::string quialified_name();

    std::string set(int cell, int size, state& ctx) override;

    std::string set_to(int cell, state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

struct num : value {
    int value;

public:
    explicit num(int value);

    std::string set_to(int cell, state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

typedef std::shared_ptr<num> sh_num_p;
typedef std::shared_ptr<var> sh_var_p;

struct arr : lvalue {
    enum type {
        GLOB,
        PRM,
        USR,
    } t = GLOB;
    sh_var_p usr_base = nullptr;
//    sh_value_p index;
    sh_num_p index;

private:
    int address(state& ctx);

public:
//    explicit arr(sh_value_p index);
    explicit arr(sh_num_p index);

    std::string set(int cell, int set, state& ctx) override;

    std::string set_to(int cell, state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

struct str : value {
    std::string value;

public:
    explicit str(std::string s);

    int alloc_sz() override;

    std::string set_to(int cell, state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

struct arry : value {
    std::vector<sh_value_p> values;

public:
    arry() = default;

    int alloc_sz() override;

    std::string set_to(int cell, state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

struct arithm : value {
    enum OP {
        PLUS,
        MINUS,
    };

    sh_value_p l;
    sh_value_p r;
    OP op;

public:
    arithm(sh_value_p  l, sh_value_p  r, OP op);

    std::string set_to(int cell, state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

struct func;

typedef std::shared_ptr<func> sh_func_p;

struct context {
    std::map<std::string, sh_var_p> variables;
    std::map<std::string, sh_func_p> functions;
    context* parent = nullptr;

public:
    explicit context(context* ctx);

    void add_var(std::string const& name, sh_var_p variable);

    void add_fun(std::string const& name, sh_func_p function);

    sh_var_p is_var(std::string const& name);

    sh_func_p is_fun(std::string const& name);

    void load_context(state& ctx);

    void free_context(state& ctx);
};

struct while_st : context, statement {
    sh_value_p condition;
    std::vector<sh_st_p> sts;

public:
    explicit while_st(context* ctx);

    std::string encode(state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

struct if_st : context, statement {
    sh_value_p condition;
    std::vector<sh_st_p> pos;
    std::vector<sh_st_p> neg;

public:
    explicit if_st(context* ctx);

    std::string encode(state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

struct set_st : statement {
    sh_lvalue_p lh;
    sh_value_p rh;

public:
    set_st(sh_lvalue_p lh, sh_value_p rh);

    std::string encode(state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

struct func : context, ast_node {
    std::string name;
    std::vector<sh_st_p> sts;

public:
    func(context* ctx, std::string name);

    [[nodiscard]] std::string to_string() const override;
};

struct func_st : value, statement {
    sh_func_p f;
    std::vector<sh_value_p> parms;

public:
    explicit func_st(sh_func_p f);

    std::string set_to(int cell, state& ctx) override;

    std::string encode(state& st) override;

    [[nodiscard]] std::string to_string() const override;
};

struct ret_st : statement {
    sh_value_p v;

public:
    explicit ret_st(sh_value_p v);

    std::string encode(state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

struct program : statement, context {
    std::vector<sh_func_p> funcs;

public:
    program();

    std::string encode(state& ctx) override;

    [[nodiscard]] std::string to_string() const override;
};

typedef std::shared_ptr<program> sh_prog_p;

#endif //TEST_AST_H
