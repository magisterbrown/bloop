#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "lexer.h"

#ifndef BLOOP_H
#define BLOOP_H

#define UNREACHABLE(message) do { std::cerr<< __FILE__ << ":" << __LINE__ << " UNREACHABLE: " << message << std::endl; abort();} while(0)
class Context {
public: 
    std::map<int, int> cells;
    std::map<std::string, int> parameters;
    int output = 0;
};

class SExpr {
public: virtual int get() = 0;
    virtual ~SExpr() = default; 
};

std::unique_ptr<SExpr> parse_expression(Lexer &lex, std::shared_ptr<Context> context); 

void report_error(Lexer &lex, Cur &position, std::string msg);
Token get_next(Lexer &lex);
Token peek_next(Lexer &lex);
void consume_type(Lexer &lex, Token expected); 
void consume_name(Lexer &lex, std::string name);

class Step {
public: 
    virtual void execute() = 0;
    virtual ~Step() = default;
};

class Block {
public: 
    Block() = default;
    Block(Lexer &lex, std::shared_ptr<Context> context);
    void execute();
private: 
    std::vector<std::unique_ptr<Step>> steps;
    int index;
};
#endif
