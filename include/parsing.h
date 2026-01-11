#include "lexer.h"

#ifndef PARSING_H
#define PARSING_H

Token get_next(Lexer &lex);
Token peek_next(Lexer &lex);
void consume_type(Lexer &lex, Token expected); 
void consume_name(Lexer &lex, std::string name);

#endif
