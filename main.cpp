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



class Step {
public: 
    void execute();
};


class Assignment : public Step {
public:
    Assignment() = default;
    Assignment(Lexer &lex, Context &context);
private:
    Context &context;
    int target_idx;
    std::unique_ptr<SExpr> expression;
};

Assignment::Assignment(Lexer &lex, Context &context) : context(context){
        consume_name(lex, "cell");
        consume_type(lex, Token::LBracket);
        consume_type(lex, Token::Digit);
        target_idx = lex.number;
        consume_type(lex, Token::RBracket);
        consume_type(lex, Token::Assign);
        expression = parse_expression(lex, context);
        context.cells[target_idx] = 0;
}
class OutputAssignment : public Step {
public:
    OutputAssignment(Lexer &lex, Context &context);
private:
    Context &context;
    std::unique_ptr<SExpr> expression;
};

OutputAssignment::OutputAssignment(Lexer &lex, Context &context) : context(context){
        consume_name(lex, "output");
        consume_type(lex, Token::Assign);
        expression = parse_expression(lex, context);
}

class Block {
public: 
    Block() = default;
    Block(Lexer &lex, Context &context);
    void execute();
private: 
    std::vector<Step> steps;
    int index;
};

class Loop : public Step{
public: 
    Loop(Lexer &lex, Context &context);
private:
    //Assignment n_times;
    std::unique_ptr<SExpr> n_times;
    Block iteration;
};


Block::Block(Lexer &lex, Context &context) {
    consume_name(lex, "block");
    consume_type(lex, Token::Digit);
    index = lex.number;
    consume_type(lex, Token::Column);
    consume_name(lex, "begin");
    for(;;) {
        Cur checkpoint = lex.cur;
        consume_type(lex, Token::Identifier);
        std::string name = lex.string;
        lex.cur = checkpoint;
        if(name.compare("cell") == 0) {
            steps.push_back(Assignment(lex, context));
        } else if(name.compare("output") == 0) {
            steps.push_back(OutputAssignment(lex, context));
        } else if(name.compare("loop") == 0) {
            steps.push_back(Loop(lex, context));
        } else if(name.compare("block") == 0) {
            break;
        }
        consume_type(lex, Token::SemiColumn);
    }
    consume_name(lex, "block");
    consume_type(lex, Token::Digit);
    if(lex.number != index) 
        abort();
    consume_type(lex, Token::Column);
    consume_name(lex, "end");
    // Finish parsing a block
}

Loop::Loop(Lexer &lex, Context &context) {
    consume_name(lex, "loop");
    n_times = parse_expression(lex, context);
    consume_name(lex, "times");
    consume_type(lex, Token::Column);
    iteration = Block(lex, context);
}

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
    for(;;) {
        Token start = peek_next(lex);
        if(start == Token::Eof)
            break;
        if(start == Token::Identifier) {
            auto proc = Procedure(lex);
        }
    }
}
