#include "parsing.h"

Token get_next(Lexer &lex) {
    bool success = lex.next_token();
    if(!success) {
        // TODO: raise parsing error   
        abort();
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
        abort();
    }
}

void consume_name(Lexer &lex, std::string name) {
    consume_type(lex, Token::Identifier);
    if(lex.string.compare(name) != 0) {
        abort();
    }
}
