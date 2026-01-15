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
    size_t pos, row, bol;
    Cur() : pos(0), row(0), bol(0) {}
};

class Lexer {
public:
    Lexer(std::string content);
    bool next_token();

    Cur cur;

    Token tok;
    int number;
    std::string string;
    
    std::string _content;
private:
    char get_char();
    char next_char();
};

std::string print_token(Token tok);
#endif
