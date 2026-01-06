#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

#include "lexer.h"

class Step {
public: 
    void execute();
}

class Procedure {
public:
    Procedure();
    int prio;
    void execute(std::map<std::string, Procedure> &procedures);
private:
    std::map<int, int> cells; 
};

Procedure::Procedure() {
    //prio = this->proc_counter++;
}

Token get_next(Lexer &lex) {
    bool success = lex.next_token();
    if(!success) {
        // TODO: raise parsing error   
        abort();
    }
    return lex.tok;
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

int main() {
    char ch;
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
    asm("int3");
    for(;;) {
        Token start = get_next(lex);
        if(start == Token::Eof)
            break;
        if(start == Token::Identifier && lex.string.compare("define") == 0) {
            consume_name(lex, "procedure");
            consume_type(lex, Token::Backticks);
            consume_type(lex, Token::Identifier);
            std::string procedure_name = lex.string;
            consume_type(lex, Token::Backticks);
            consume_type(lex, Token::LSquareBracket);
            std::map<std::string, int> parameters;
            for(int i=0;;i++) {
                consume_type(lex, Token::Identifier);
                parameters[lex.string] = i;
                Token next = get_next(lex);
                if(next == Token::RSquareBracket) 
                    break;
                if(next != Token::Comma)
                    abort();
            } 
            consume_type(lex, Token::Column);
        }
        break;
    }
}
