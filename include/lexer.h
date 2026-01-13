#include <string>

#ifndef LEX_H
#define LEX_H

enum class Token
{
    Identifier,
    Backticks,
    LSquareBracket,
    RSquareBracket,
    LBracket,
    RBracket,
    Assign,
    Multiply,
    Plus,
    Column,
    SemiColumn,
    Eof,
    None,
    Digit,
    Comma,
    Dot,
};


struct Cur {
    size_t pos, row;
    Cur() : pos(0), row(0) {}
};

class Lexer {
public:
    Lexer(std::string content);
    bool next_token();
    std::string print_token();

    Cur cur;

    Token tok;
    int number;
    std::string string;
    
private:
    std::string _content;
    char get_char();
    char next_char();
};

#endif
