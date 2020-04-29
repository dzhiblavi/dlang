#ifndef TEST_TRANS_H
#define TEST_TRANS_H

#include <string>
#include <stack>
#include <map>

// user space: [0, 100)
// base cell: 100. each operation must return pointer to this cell
// reserved: [100, 110)
// stack: [110, 300)
const int BASE_CELL = 100;
const int USER = 0;
const int STACK = 110;

struct state {
private:
    std::stack<int> st;
    std::map<std::string, std::stack<int>> var_addrs;
    int stck = STACK;
    int base = 0;
    std::stack<int> retst;
    std::stack<int> parmst;

public:
    int nextalloc();

    int stalloc(int sz = 1);

    void stdealloc(int cell, int sz = 1);

    void retalloc();

    void retdealloc(int cell);

    int retcell();

    void parmalloc(int cnt);

    void parmdealloc(int cell, int cnt);

    int parmcell();

    void force_setparm(int cell);

    void force_rmparm(int cell);

    void var_alloc(std::string const& name);

    int var_addr(std::string const& name);

    void var_free(std::string const& name);

    std::string radd(int cell, int x);

    std::string copy(int from, int to, int tmp);

    std::string add(int from, int to);

    std::string sub(int from, int to);

    std::string mov(int from, int to);

    std::string set(int cell, int val);

    std::string zero(int cell);

    std::string sh(int cnt);
};

#endif //TEST_TRANS_H
