#include "dtransmt.h"

std::vector<std::pair<char, char>> dtransmt::st_ranges = {
        { '0', '1' },
};

dtransmt::dtransmt(sh_p_p const &ast)
    : ast(ast) {}

void dtransmt::append_all_file(std::string& s, std::filesystem::path const& p) {
    std::ifstream ifs(p);
    s.append((std::istreambuf_iterator<char>(ifs))
            , std::istreambuf_iterator<char>());
}

void dtransmt::load_native(std::string &s) {
    std::filesystem::path native_path("../mt_native");
    for (auto& p : std::filesystem::directory_iterator(native_path)) {
        if (p.path().extension() != ".mtt") {
            std::ifstream ifs(p);
            load_native_template(s, ifs, "", "");
            s += "\n\n";
        }
    }
}

void dtransmt::postprocess(std::string& s) {
    std::stringstream ss(s);
    s = "";
    load_native_template(s, ss, "", "");
}

void dtransmt::load_native_template(std::string& rss, std::istream& ss, std::string const& uid, std::string const& ret) {
    std::string line;

    while (getline(ss, line)) {
        for (int i = 0; i < line.size(); ++i) {
            if (line[i] == '@') {
                line = line.replace(i, 1, uid);
            } else if (line[i] == '&') {
                line = line.replace(i, 1, ret);
            }
        }

        if (line[0] == '%') {
            continue;
        } else if (line == "") {
            rss += "\n";
            continue;
        }

        std::stringstream ls(line);
        std::string word;
        ls >> word;

        if (word.back() != ':') {
            std::string arrow, tow;
            char c, a, d;
            ls >> c >> arrow >> tow >> a >> d;
            assert(arrow == "->");

            std::vector<std::string> r = { line };
            std::map<char, bool> templates = {
                    { '*', false },
                    { '~', false }
            };

            for (char ch : word) {
                if (templates.find(ch) != templates.end()) {
                    templates[ch] = true;
                }
            }
            if (templates.find(c) != templates.end()) {
                templates[c] = true;
            }

            for (auto const& p : templates) {
                if (p.second) {
                    std::vector<std::string> rnew;
                    char t = p.first;

                    for (auto& l : r) {
                        std::vector<int> pos;
                        for (int i = 0; i < l.size(); ++i)
                            if (l[i] == t)
                                pos.push_back(i);

                        for (auto const& range : st_ranges) {
                            for (char cn = range.first; cn <= range.second; ++cn) {
                                for (int i : pos) {
                                    l[i] = cn;
                                }

                                rnew.push_back(l);

                                for (int i : pos) {
                                    l[i] = t;
                                }
                            }
                        }
                    }

                    r = std::move(rnew);
                }
            }

            for (auto const& l : r) {
                rss += l + "\n";
            }
        } else {
            rss += line + "\n";
        }
    }
}

void dtransmt::load_ast(std::string& s) {
    sh_f_p f = ast->funcs["main"];

    std::string return_point = "__return_call_main";
    f->compile("", return_point, s);

    std::string ur = "_main";
    std::string native_return = "__native_return_" + ur;
    s += return_point + " * -> " + native_return + " * ^\n\n";
    s += return_point + " $ -> " + native_return + " $ ^\n\n";
    s += return_point + " _ -> " + native_return + " _ >\n\n";

    std::ifstream ifs("../mt_native/return_native.mtt");
    dtransmt::load_native_template(s, ifs, ur, "__native_fin");
}

std::string dtransmt::compile() {
    std::string code;
    load_native(code);
    load_ast(code);
    postprocess(code);
    return code;
}
