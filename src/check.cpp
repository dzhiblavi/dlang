#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

static char cpu[30000];

int main(int argc, char **argv) {
    vector<char> acc;
    char ch;
    ifstream infile("../out.txt");
    while (infile) {
        infile.get(ch);
        acc.push_back(ch);
    }

    infile.close();
    unsigned int j = 0;
    int brc = 0;
    for (int i = 0; i < acc.size(); ++i) {
        if (acc[i] == '>')
            j++;
        if (acc[i] == '<')
            j--;
        if (acc[i] == '+')
            cpu[j]++;
        if (acc[i] == '-')
            cpu[j]--;
        if (acc[i] == '.') {
//            cout << i << ' ' << acc[i] << endl;
            cout << cpu[j];
        }
        if (acc[i] == ',')
            cin >> cpu[j];
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
            } else
                continue;
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