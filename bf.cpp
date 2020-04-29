#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

static int cpu[30000];

int main(int argc, char **argv) {
    ifstream infile("../out.txt");
    std::string acc((std::istreambuf_iterator<char>(infile))
            , std::istreambuf_iterator<char>());

    unsigned int j = 0;
    int brc = 0;
    for (int i = 0; i < acc.size(); ++i) {
        if (acc[i] == '>') {
//            std::cout << "s >" << std::endl;
            j++;
        }
        if (acc[i] == '<') {
//            std::cout << "s <" << std::endl;
            j--;
        }
        if (acc[i] == '+') {
//            std::cout << "+ #" << j << std::endl;
            cpu[j]++;
        }
        if (acc[i] == '-') {
//            std::cout << "- #" << j << std::endl;
            cpu[j]--;
        }
        if (acc[i] == '.') {
//            std::cout << "print #" << j << ' ' << (int)cpu[j] << std::endl;
            cout << (char)cpu[j];
        }
        if (acc[i] == ',') {
            char c;
            cin >> c;
            cpu[j] = (int)c;
        }
        if (acc[i] == '[') {
            if (!cpu[j]) {
                ++brc;
                while (brc) {
                    ++i;
                    if (acc[i] == '[')
                        ++brc;
                    if (acc[i] == ']')
                        --brc;
                }
            } else {
                continue;
            }
        } else if (acc[i] == ']') {
            if (!cpu[j])
                continue;
            else {
                if (acc[i] == ']')
                    brc++;
                while (brc) {
                    --i;
                    if (acc[i] == '[')
                        brc--;
                    if (acc[i] == ']')
                        brc++;
                }
                --i;
            }
        }
        if (j < 0) {
            std::cout << "< 0" << std::endl;
        } else if (j >= 30000) {
            std::cout << "> 30k" << std::endl;
        }
        assert(j >= 0 && j < 30000);
    }
}