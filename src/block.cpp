#include <memory>
#include "bloop.h"

class Assignment : public Step {
public:
    Assignment() = default;
    Assignment(Lexer &lex, std::shared_ptr<Context> context);
    void execute() override;
private:
    std::shared_ptr<Context> context;
    int target_idx;
    std::unique_ptr<SExpr> expression;
};

Assignment::Assignment(Lexer &lex, std::shared_ptr<Context> context) : context(context){
        consume_name(lex, "cell");
        consume_type(lex, Token::LBracket);
        consume_type(lex, Token::Digit);
        target_idx = lex.number;
        consume_type(lex, Token::RBracket);
        consume_type(lex, Token::Assign);
        expression = parse_expression(lex, context);
        context->cells[target_idx] = 0;
}

void Assignment::execute() {
    context->cells[target_idx] = expression->get();
}

class OutputAssignment : public Step {
public:
    OutputAssignment(Lexer &lex, std::shared_ptr<Context> context);
    void execute() override;
private:
    std::shared_ptr<Context> context;
    std::unique_ptr<SExpr> expression;
};

OutputAssignment::OutputAssignment(Lexer &lex, std::shared_ptr<Context> context) : context(context){
        consume_name(lex, "output");
        consume_type(lex, Token::Assign);
        expression = parse_expression(lex, context);
}

void OutputAssignment::execute() {
    context->output = expression->get();
}

class Loop : public Step{
public: 
    Loop(Lexer &lex, std::shared_ptr<Context> context);
    void execute() override;
private:
    std::unique_ptr<SExpr> n_times;
    Block iteration;
};

Loop::Loop(Lexer &lex, std::shared_ptr<Context> context) {
    consume_name(lex, "loop");
    n_times = parse_expression(lex, context);
    consume_name(lex, "times");
    consume_type(lex, Token::Column);
    iteration = Block(lex, context);
}

void Loop::execute() {
    int n = n_times->get();
    for(int i=0;i<n;i++) {
        iteration.execute();
    }
}

//class IfStatement : public Step {
//public: 
//    IfStatement(Lexer &lex, std::shared_ptr<Context> context);
//    void execute() override;
//};
//
//IfStatement::IfStatement(Lexer &lex, std::shared_ptr<Context> context) {
//
//}

std::unique_ptr<Step> parse_single_step(Lexer &lex, std::shared_ptr<Context> context) {
   Cur checkpoint = lex.cur;
   consume_type(lex, Token::Identifier);
   Cur last = lex.cur;
   std::string name = lex.string;
   lex.cur = checkpoint;
   if(name.compare("cell") == 0) {
       return std::make_unique<Assignment>(lex, context);
   } else if(name.compare("output") == 0) {
       return std::make_unique<OutputAssignment>(lex, context);
//   } else if(name.compare("if") == 0) {
//       return std::make_unique<IfStatement>(lex, context);
   } else if(name.compare("loop") == 0) {
       return std::make_unique<Loop>(lex, context);
   } else if(name.compare("block") == 0) {
       consume_name(lex, "block");
       consume_type(lex, Token::Digit);
       consume_type(lex, Token::Column);
       consume_type(lex, Token::Identifier);
       if(lex.string.compare("end") == 0) {
           lex.cur = checkpoint;
           return 0;
       }
       return std::make_unique<Block>(lex, context);
   } 
   report_error(lex, last, "Expected cell, output, loop or block on this line"); 
}


Block::Block(Lexer &lex, std::shared_ptr<Context> context) {
    consume_name(lex, "block");
    consume_type(lex, Token::Digit);
    index = lex.number;
    consume_type(lex, Token::Column);
    consume_name(lex, "begin");
    for(;;) {
        std::unique_ptr<Step> next = parse_single_step(lex, context);
        if(!next)
            break;
        consume_type(lex, Token::SemiColumn);
    }
    consume_name(lex, "block");
    consume_type(lex, Token::Digit);
    if(lex.number != index) 
        report_error(lex, lex.cur, "Expected end of the block: " + std::to_string(index));
    consume_type(lex, Token::Column);
    consume_name(lex, "end");
}

void Block::execute() {
    for(auto &step : steps)
        step->execute();
}


