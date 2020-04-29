#ifndef TEST_DLEXER_H
#define TEST_DLEXER_H

#include <string>
#include <list>
#include <vector>
#include <stack>
#include <iostream>

#include "stringref.h"

enum TOKEN {
    END,
    RET,        // return
    QT,         // "
    QS,         // '
    COMMA,      // ,
    POP,        // (
    PCL,        // )
    COP,        // {
    CCL,        // }
    SOP,        // [
    SCL,        // ]
    WHILE,      // while
    IF,         // if
    ELSE,       // else
    EQ,         // =
    GR,         // >
    PLUS,       // +
    MINUS,      // -
    IDENT,      // identifier, (a-Z)+
    NUM,        // integral value, 0-255
    UNKN,
};

class dlexer {
public:
    typedef std::pair<TOKEN, std::string> token;

private:
    stringref s;
    std::stack<token> back;

private:
    bool tf(stringref& tkn, std::string const& ls);

    bool tf(stringref& tkn, int (*f) (int));

    void skip_ws();

public:
    explicit dlexer(stringref const& sr);

    token next_token();

    token lookup();

    void putback(token t);

    void skip(size_t n = 1);
};




#endif //TEST_DLEXER_H
