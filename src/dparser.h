#ifndef TEST_DPARSER_H
#define TEST_DPARSER_H

#include <string>
#include <vector>
#include <map>

#include "dlexer.h"
#include "ast.h"

class dparser {
    dlexer l;

private:
    void assert_next(TOKEN tkn);

    std::string parse_identifier();

    sh_st_p parse_while(context* ctx);

    sh_st_p parse_if(context* ctx);

    std::unique_ptr<func_st> parse_fun_st(context* ctx);

    sh_st_p parse_set(context* ctx);

    void parse_fun_call(context* ctx, std::vector<sh_value_p>& sts);

    sh_value_p parse_string();

    std::unique_ptr<num> parse_num();

    sh_value_p parse_array(context* ctx);

    sh_var_p parse_variable(context* ctx, bool defined);

    sh_value_p parse_char();

    std::unique_ptr<arr> parse_array_subscript(context* ctx);

    sh_value_p parse_expr(context* ctx);

    sh_lvalue_p parse_lvalue(context* ctx, bool defined);

    sh_value_p parse_value(context* ctx);

    sh_st_p parse_statement(context* ctx);

    void parse_block(context* ctx, std::vector<sh_st_p>& sts);

    sh_func_p parse_function(context* ctx);

public:
    explicit dparser(dlexer lexer);

    sh_prog_p parse();
};

#endif //TEST_DPARSER_H
