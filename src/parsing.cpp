#include <iostream>
#include "bloop.h"

void report_error(Lexer &lex, const Cur &position, std::string msg) {
    std::string line = lex._content.substr(position.bol);
    int col = position.pos-1-position.bol;
    std::cerr << "Error on line: " << position.row << " Column: " << col << ":" << std::endl;
    std::cerr << line.substr(0, line.find('\n')) << std::endl;
    std::cerr << std::string(col, ' ') << "^" << std::endl;
    std::cerr << std::string(col, ' ') << msg << '.' << std::endl;
    abort();
}

Token get_next(Lexer &lex) {
    bool success = lex.next_token();
    if(!success) {
        report_error(lex, lex.cur, "Is not a valid token");
    }
    return lex.tok;
}

Token peek_next(Lexer &lex) {
    Cur save = lex.cur;
    Token res = get_next(lex);
    lex.cur = save;
    return res;
}

void consume_type(Lexer &lex, Token expected) {
    Token actual = get_next(lex);
    if(actual != expected) {
        report_error(lex, lex.cur, "Expected token: "+ print_token(expected) + " Got: " +print_token(actual));
    }
}

void consume_name(Lexer &lex, std::string name) {
    consume_type(lex, Token::Identifier);
    if(lex.string.compare(name) != 0) {
        report_error(lex, lex.cur, "Expected identifier: "+ name  + " Got: " +lex.string);
    }
}
