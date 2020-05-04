#ifndef DLANG_DTRANSMT_H
#define DLANG_DTRANSMT_H

#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

#include "ast.h"

class dtransmt {
    sh_p_p ast;

    static std::vector<std::pair<char, char>> st_ranges;

private:
    static void append_all_file(std::string& s, std::filesystem::path const& p);

    static void postprocess(std::string& s);

    static void load_native(std::string& s);

    void load_ast(std::string& s);

public:
    dtransmt(sh_p_p const& ast);

    std::string compile();

    static void load_native_template(std::string& rss, std::istream& ss, std::string const& uid, std::string const& ret);
};


#endif //DLANG_DTRANSMT_H
