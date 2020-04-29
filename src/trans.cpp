#include "trans.h"

int state::nextalloc() {
    return stck;
}

int state::stalloc(int sz) {
    int ret = stck;
    while (sz--) {
        st.push(stck++);
    }
    return ret;
}

void state::stdealloc(int cell, int sz) {
    while (sz--) {
        assert(!st.empty() && st.top() == cell + sz);
        st.pop();
        --stck;
    }
}

void state::retalloc() {
    retst.push(stalloc());
}

void state::retdealloc(int cell) {
    assert(!retst.empty() && retst.top() == cell);
    stdealloc(retst.top());
    retst.pop();
}

int state::retcell() {
    assert(!retst.empty());
    return retst.top();
}

void state::parmalloc(int cnt) {
    int addr = stalloc(cnt);
    parmst.push(addr);
}

void state::parmdealloc(int cell, int cnt) {
    assert(!parmst.empty() && parmst.top() == cell);
    parmst.pop();
    stdealloc(cell, cnt);
}

int state:: parmcell() {
    assert(!parmst.empty());
    return parmst.top();
}

void state::force_setparm(int cell) {
    parmst.push(cell);
}

void state::force_rmparm(int cell) {
    assert(!parmst.empty() && parmst.top() == cell);
    parmst.pop();
}

void state::var_alloc(const std::string &name) {
    auto p = var_addrs.find(name);
    if (p == var_addrs.end()) {
        var_addrs[name] = {};
    }
    var_addrs[name].push(stalloc());
}

int state::var_addr(const std::string &name) {
    assert(var_addrs.find(name) != var_addrs.end());
    return var_addrs[name].top();
}

void state::var_free(const std::string &name) {
    auto p = var_addrs.find(name);
    assert(p != var_addrs.end());
    stdealloc(p->second.top());
    var_addrs[name].pop();
}

std::string state::zero(int cell) {
    std::string code;
    code += sh(cell);
    code += "[-]";
    return code;
}

std::string state::copy(int from, int to, int tmp) {
    std::string code;
    code += zero(to);
    code += zero(tmp);
    code += sh(from);
    code += "[-";
    code += sh(to);
    code += "+";
    code += sh(tmp);
    code += "+";
    code += sh(from);
    code += "]";
    code += mov(tmp, from);
    return code;
}

std::string state::radd(int cell, int x) {
    std::string code;
    code += sh(cell);
    while (x > 0) {
        --x;
        code += "+";
    }
    while (x < 0) {
        ++x;
        code += "-";
    }
    return code;
}

std::string state::add(int from, int to) {
    assert(from != to);
    std::string code;
    code += sh(from);
    code += "[-";
    code += sh(to);
    code += "+";
    code += sh(from);
    code += "]";
    return code;
}

std::string state::sub(int from, int to) {
    assert(from != to);
    std::string code;
    code += sh(from);
    code += "[-";
    code += sh(to);
    code += "-";
    code += sh(from);
    code += "]";
    return code;
}

std::string state::mov(int from, int to) {
    std::string code;
    code += zero(to);
    code += add(from, to);
    return code;
}

std::string state::set(int cell, int val) {
    std::string code;
    code += zero(cell);
    code += radd(cell, val);
    return code;
}

std::string state::sh(int cell) {
    int shift = cell - base;
    base = cell;

    std::string r;
    while (shift > 0) {
        --shift;
        r += ">";
    }
    while (shift < 0) {
        ++shift;
        r += "<";
    }
    return r;
}