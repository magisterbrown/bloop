#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <memory>
#include <set>

#include "lexer.h"
#include "bloop.h"

class Procedure {
public:
    Procedure() = default;
    Procedure(Lexer &lex);
    int prio;
    int execute(std::vector<int> args);
    std::string name;
private:
    Cur definition;
    std::shared_ptr<Context> context; 
    std::map<int, std::string> positions;
    Block bl;
};

std::set<std::string> reserved_names = {"define", "procedure", "block", "begin", "times", "loop", "cell", "end", "output", "at", "most", "quit", "abort"};

Procedure::Procedure(Lexer &lex) : context(std::make_shared<Context>()) {
    consume_name(lex, "define");
    consume_name(lex, "procedure");
    consume_type(lex, Token::Backticks);
    consume_type(lex, Token::Identifier);
    name = lex.string;
    consume_type(lex, Token::Backticks);
    consume_type(lex, Token::LSquareBracket);
    definition = lex.cur;
    for(int i=0;;i++) {
        consume_type(lex, Token::Identifier);
        if(reserved_names.count(lex.string))
            report_error(lex, lex.cur, "Word: '" + lex.string + "' is reserved and can't be used as variable name");
        context->parameters[lex.string] = 0;
        positions[i] = lex.string;
        Token next = get_next(lex);
        if(next == Token::RSquareBracket) 
            break;
        if(next != Token::Comma)
            report_error(lex, lex.cur, "Expected next argument separated by ','");
    } 
    consume_type(lex, Token::Column);
    bl = Block(lex, context);
    consume_type(lex, Token::Dot);
}

int Procedure::execute(std::vector<int> args) {
    if(args.size() != positions.size())
        throw ExecutionError("Expected: " + std::to_string(positions.size()) + " arguments, but got: " + std::to_string(args.size()) + " arguments", definition);
    context->output = 0; 
    for(auto &[key, cell]: context->cells)
        cell=0; 
    for(size_t i=0;i<args.size();i++) {
        std::string name = positions[i];
        context->parameters[name] = args[i];
    }
    bl.execute();
    return context->output;
}


int main(int argc, char **argv) {
    std::string source_code;
    {
        std::stringstream content;
        std::ifstream input(argv[1]);
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
    for(int i=2;i<argc;i++) {
        args.push_back(std::stoi(argv[i]));
    }
    try {
        int res = last.execute(args);
        std::cout << last.name << " " << res << std::endl;
    } catch(const ExecutionError &e) {
        report_error(lex, e.point, e.msg);
    }
}
