#include <stack>

#include "optimizer.h"

void optimizer::optimize(std::string &s) {
    std::deque<char> st;

    for (char c : s) {
        if (c == '<') {
            if (!st.empty() && st.back() == '>') {
                st.pop_back();
            } else {
                st.push_back(c);
            }
        } else if (c == '>') {
            if (!st.empty() && st.back() == '<') {
                st.pop_back();
            } else {
                st.push_back(c);
            }
        } else {
            st.push_back(c);
        }
    }

    s = std::string(st.begin(), st.end());
}
