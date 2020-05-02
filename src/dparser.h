#ifndef DLANG_DPARSER_H
#define DLANG_DPARSER_H

#include <set>

#include "dlexer.h"
#include "ast.h"

class dparser {
    dlexer l;
    std::map<std::string, sh_f_p> fs;

private:
    void assert_next(TOKEN t);

    std::map<std::string, int> parse_args_declaration(int nargs);

    std::unique_ptr<str> parse_str_value();

    std::unique_ptr<call> parse_call_value(std::map<std::string, int>& args);

    std::unique_ptr<arg> parse_arg_value(std::map<std::string, int>& args);

    std::unique_ptr<chv> parse_chv_value();

    sh_v_p parse_value(std::map<std::string, int>& args);

    int parse_int();

    sh_f_p parse_declaration();

    void parse_definition();

public:
    dparser(dlexer& l);

    sh_p_p parse();
};


#endif //DLANG_DPARSER_H
