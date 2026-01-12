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
    Procedure() = default;
    Procedure(Lexer &lex);
    int prio;
    int execute(std::vector<int> args);
private:
    Context context; 
    std::map<int, std::string> positions;
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
        positions[i] = lex.string;
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

int Procedure::execute(std::vector<int> args) {
    if(args.size() != positions.size())
        abort();
    context.output = 0; 
    for(auto &[key, cell]: context.cells)
        cell=0; 
    for(size_t i=0;i<args.size();i++) {
        std::string name = positions[i];
        context.parameters[name] = args[i];
    }
    bl.execute();
    return context.output;
}


int main(int argc, char **argv) {
    std::string source_code;
    {
        std::stringstream content;
        std::ifstream input("to_the_n.bloop");
        content << input.rdbuf();
        source_code = content.str();
    }
    Lexer lex = Lexer(source_code);
    Procedure last;
    for(;;) {
        Token start = peek_next(lex);
        if(start == Token::Eof)
            break;
        if(start == Token::Identifier) {
            last = Procedure(lex);
        }
    }
    std::vector<int> args;
    for(int i=1;i<argc;i++) {
        args.push_back(std::stoi(argv[i]));
    }
    last.execute(args);
}
