#include <iostream>
#include <vector>
#include <fstream>

#include "src/dlexer.h"
#include "src/dparser.h"


using namespace std;

int main() {
    ifstream fin("../main.d");
    ofstream fout("../out.txt");

    std::string s((std::istreambuf_iterator<char>(fin))
            , std::istreambuf_iterator<char>());

    dlexer l(s);
    dparser d(l);

    try {
        sh_p_p prog = d.parse();
        std::cout << prog->to_string() << std::endl;
    } catch (std::runtime_error const& e) {
        std::cerr << "failed: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "failed" << std::endl;
    }
    return 0;
}