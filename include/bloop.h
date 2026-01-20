#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <set>
#include "lexer.h"

#ifndef BLOOP_H
#define BLOOP_H

#define UNREACHABLE(message) do { std::cerr<< __FILE__ << ":" << __LINE__ << " UNREACHABLE: " << message << std::endl; abort();} while(0)
class Block;
class Procedure;
class ParsingContext;

class Context {
public: 
    std::map<int, int> cells;
    std::map<std::string, int> parameters;
    int output = 0;
    std::set<int> blocks;
    std::set<int> abortable;
};


class SExpr {
public: 
    virtual int get() = 0;
    virtual ~SExpr() = default;
};


class StepResult {
public:
    enum class Value { Quit, Abort, Continue};
    Value res;
    int block_index = -1;
    StepResult() : res(Value::Continue) {};
    StepResult(Value res, int block_index) : res(res), block_index(block_index) {};
};

class Step {
public: 
    virtual StepResult execute() = 0;
    virtual ~Step() = default;
};

class Block : public Step {
public: 
    Block() = default;
    Block(Lexer &lex, std::shared_ptr<Context> context, ParsingContext &parsc);
    StepResult execute();
    int index;
private: 
    std::vector<std::unique_ptr<Step>> steps;
};

class Procedure {
public:
    Procedure() = default;
    Procedure(Lexer &lex, std::map<std::string, std::shared_ptr<Procedure>> &defined);
    int prio;
    int execute(std::vector<int> args);
    std::string name;
    Procedure(Procedure &&other) = default;
    Procedure &operator =(Procedure &&other) = default;
    std::shared_ptr<Context> context; 
    bool is_test;
private:
    Cur definition;
    std::map<int, std::string> positions;
    Block bl;
};

class ParsingContext {
public: 
    ParsingContext(std::map<std::string, std::shared_ptr<Procedure>> &defined) : defined(defined) {};
    std::set<int> blocks;
    std::set<int> abortable;
    std::map<std::string, std::shared_ptr<Procedure>> &defined;
};

class ExecutionError : public std::exception {
public: 
    std::string msg;
    Cur point;
    ExecutionError(std::string msg, Cur point) : msg(msg), point(point) {}
};

std::unique_ptr<SExpr> parse_expression(Lexer &lex, std::shared_ptr<Context> context, ParsingContext &parsc); 

void report_error(Lexer &lex, const Cur &position, std::string msg);
Token get_next(Lexer &lex);
Token peek_next(Lexer &lex);
void consume_type(Lexer &lex, Token expected); 
void consume_name(Lexer &lex, std::string name);

#endif
