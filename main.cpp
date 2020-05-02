#include <iostream>
#include <vector>
#include <fstream>

#include "src/dparser.h"
#include "src/optimizer.h"

using namespace std;

int main() {
    ifstream fin("../main.d");
    ofstream fout("../out.txt");

    std::string s((std::istreambuf_iterator<char>(fin))
            , std::istreambuf_iterator<char>());

    dlexer d(s);
    dparser p(d);
    sh_prog_p root = nullptr;

    try {
        root = p.parse();
        std::cout << root->to_string() << std::endl;
        state st;
        std::string code = root->encode(st);
        optimizer::optimize(code);
        fout << code;
    } catch (std::runtime_error const& e) {
        std::cerr << "failed: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "failed" << std::endl;
    }
    return 0;
}