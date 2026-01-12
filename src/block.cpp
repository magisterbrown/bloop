#include <memory>
#include "block.h"
#include "expression.h"
#include "parsing.h"

class Assignment : public Step {
public:
    Assignment() = default;
    Assignment(Lexer &lex, Context &context);
    void execute() override;
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

void Assignment::execute() {
}

class OutputAssignment : public Step {
public:
    OutputAssignment(Lexer &lex, Context &context);
    void execute() override;
private:
    Context &context;
    std::unique_ptr<SExpr> expression;
};

OutputAssignment::OutputAssignment(Lexer &lex, Context &context) : context(context){
        consume_name(lex, "output");
        consume_type(lex, Token::Assign);
        expression = parse_expression(lex, context);
}

void OutputAssignment::execute() {
}

class Loop : public Step{
public: 
    Loop(Lexer &lex, Context &context);
    void execute() override;
private:
    std::unique_ptr<SExpr> n_times;
    Block iteration;
};

Loop::Loop(Lexer &lex, Context &context) {
    consume_name(lex, "loop");
    n_times = parse_expression(lex, context);
    consume_name(lex, "times");
    consume_type(lex, Token::Column);
    iteration = Block(lex, context);
}

void Loop::execute() {
}

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
            //steps.push_back(Assignment(lex, context));
        } else if(name.compare("output") == 0) {
            //steps.push_back(OutputAssignment(lex, context));
        } else if(name.compare("loop") == 0) {
            //steps.push_back(Loop(lex, context));
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

void Block::execute() {

}


