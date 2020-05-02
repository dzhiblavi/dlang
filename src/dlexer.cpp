#include "dlexer.h"

namespace {
int isdigoralp(int c) {
    return std::isdigit(c) || std::isalpha(c);
}
}

bool dlexer::tf(stringref& tkn, std::string const& ls) {
    tkn = s.try_feed(ls);
    return !tkn.empty();
}

bool dlexer::tf(stringref& tkn, int (*f) (int)) {
    tkn = s.try_feed(f);
    return !tkn.empty();
}

void dlexer::skip_ws() {
    s.try_feed([] (char c) { return c == '\t' || c == '\n' || c == ' '; });
}

dlexer::dlexer(stringref const& sr)
    : s(sr), base(sr) {}

dlexer::token dlexer::next_token() {
    if (!back.empty()) {
        token r = back.top();
        back.pop();
        return r;
    }

    skip_ws();
    stringref token;

    if (tf(token, std::isdigit)) {
        return { NUM, token.str() };
    } else if (tf(token, isdigoralp)) {
        if (token == "if") {
            return { IF, token.str() };
        } else if (token == "else") {
            return { ELSE, token.str() };
        }
        return { IDENT, token.str() };
    } else if (tf(token, "+")) {
        return { PLUS, token.str() };
    } else if (tf(token, "-")) {
        return { MINUS, token.str() };
    } else if (tf(token, "=")) {
        return { EQ, token.str() };
    } else if (tf(token, "(")) {
        return { POP, token.str() };
    } else if (tf(token, ")")) {
        return { PCL, token.str() };
    } else if (tf(token, "[")) {
        return { SOP, token.str() };
    } else if (tf(token, "]")) {
        return { SCL, token.str() };
//    } else if (tf(token, "}")) {
//        return { CCL, token.str() };
//    } else if (tf(token, "{")) {
//        return { COP, token.str() };
    } else if (tf(token, "\"")) {
        return { QT, token.str() };
    } else if (tf(token, "'")) {
        return { QS, token.str() };
    } else if (tf(token, ",")) {
        return {COMMA, token.str()};
//    } else if (tf(token, ">")) {
//        return { GR, token.str() };
    } else if (tf(token, ":")) {
        return { COLON, token.str() };
    } else if (!s.empty()) {
        std::string ss;
        ss += s[0];
        s.advance();
        return { UNKN, ss };
    }
    return { END, "" };
}

dlexer::token dlexer::lookup() {
    token t = next_token();
    putback(t);
    return t;
}

void dlexer::putback(token t) {
    back.push(t);
}

void dlexer::skip(size_t n) {
    while (n-- > 0) {
        next_token();
    }
}

void dlexer::reset() {
    back = std::stack<token>();
    s = base;
}

