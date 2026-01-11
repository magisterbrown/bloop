#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <memory>

#include "lexer.h"
#include "context.h"
#include "expression.h"
#include "parsing.h"
#include "block.h"

class Procedure {
public:
    Procedure(Lexer &lex);
    int prio;
    void execute();
private:
    Context context; 
    Block bl;
};

Procedure::Procedure(Lexer &lex) {
    consume_name(lex, "define");
    consume_name(lex, "procedure");
    consume_type(lex, Token::Backticks);
    consume_type(lex, Token::Identifier);
    std::string procedure_name = lex.string;
    consume_type(lex, Token::Backticks);
    consume_type(lex, Token::LSquareBracket);
    for(int i=0;;i++) {
        consume_type(lex, Token::Identifier);
        //TODO: check for illegal var names
        context.parameters[lex.string] = 0;
        Token next = get_next(lex);
        if(next == Token::RSquareBracket) 
            break;
        if(next != Token::Comma)
            abort();
    } 
    consume_type(lex, Token::Column);
    bl = Block(lex, context);
    consume_type(lex, Token::Dot);
}


int main() {
    //std::vector<Token> tokens;
    std::string source_code;
    {
        std::stringstream content;
        std::ifstream input("to_the_n.bloop");
        content << input.rdbuf();
        source_code = content.str();
    }
    //std::map<std::string, Procedure> procedures;
    Lexer lex = Lexer(source_code);
    for(;;) {
        Token start = peek_next(lex);
        if(start == Token::Eof)
            break;
        if(start == Token::Identifier) {
            auto proc = Procedure(lex);
        }
    }
}
