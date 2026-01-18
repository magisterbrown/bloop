#include <memory>
#include "bloop.h"

class Exit : public Step {
public:
    Exit(StepResult res) : res(res) {};
    StepResult execute() override { return res;};
private:
    StepResult res;
};


class Assignment : public Step {
public:
    Assignment() = default;
    Assignment(Lexer &lex, std::shared_ptr<Context> context);
    StepResult execute() override;
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

StepResult Assignment::execute() {
    context->cells[target_idx] = expression->get();
    return StepResult();
}

class OutputAssignment : public Step {
public:
    OutputAssignment(Lexer &lex, std::shared_ptr<Context> context);
    StepResult execute() override;
private:
    std::shared_ptr<Context> context;
    std::unique_ptr<SExpr> expression;
};

OutputAssignment::OutputAssignment(Lexer &lex, std::shared_ptr<Context> context) : context(context){
        consume_name(lex, "output");
        consume_type(lex, Token::Assign);
        expression = parse_expression(lex, context);
}

StepResult OutputAssignment::execute() {
    context->output = expression->get();
    return StepResult();
}

class Loop : public Step{
public: 
    Loop(Lexer &lex, std::shared_ptr<Context> context);
    StepResult execute() override;
private:
    bool abortable = false;
    std::unique_ptr<SExpr> n_times;
    Block iteration;
};

Loop::Loop(Lexer &lex, std::shared_ptr<Context> context) {
    consume_name(lex, "loop");
    if(peek_next(lex) == Token::Identifier && lex.string.compare("at") == 0) {
        consume_name(lex, "at");
        consume_name(lex, "most");
        abortable = true;
    }
    n_times = parse_expression(lex, context);
    consume_name(lex, "times");
    consume_type(lex, Token::Column);
    iteration = Block(lex, context);
}

StepResult Loop::execute() {
    int n = n_times->get();
    for(int i=0;i<n;i++) {
        StepResult res = iteration.execute();
        if(res.res == StepResult::Value::Abort)
            if(res.block_index == iteration.index)
                break;
            else
                return res;
        if(res.res == StepResult::Value::Quit)
            if(res.block_index == iteration.index)
                continue;
            else
                return res;

    }
    return StepResult();
}

class IfStatement : public Step {
public: 
    IfStatement(Lexer &lex, std::shared_ptr<Context> context);
    StepResult execute() override;
private:
    std::unique_ptr<SExpr> condition;
    std::unique_ptr<Step> step;
};

StepResult IfStatement::execute() {
    int res = condition->get();
    if(res>0)
        return step->execute();
    return StepResult();
}

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
   } else if(name.compare("quit") == 0) {
       consume_name(lex, "quit");
       consume_name(lex, "block");
       consume_type(lex, Token::Digit);
       return std::make_unique<Exit>(StepResult(StepResult::Value::Quit, lex.number));
   } else if(name.compare("abort") == 0) {
       consume_name(lex, "abort");
       consume_name(lex, "loop");
       consume_type(lex, Token::Digit);
       return std::make_unique<Exit>(StepResult(StepResult::Value::Abort, lex.number));
   } else if(name.compare("if") == 0) {
       return std::make_unique<IfStatement>(lex, context);
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

IfStatement::IfStatement(Lexer &lex, std::shared_ptr<Context> context) {
    consume_name(lex, "if");
    condition = parse_expression(lex, context);
    consume_type(lex, Token::Comma);
    consume_name(lex, "then");
    consume_type(lex, Token::Column);
    step = parse_single_step(lex, context);
    if(!step)
        report_error(lex, lex.cur, "If should be follwed by execution step or block");
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
        steps.push_back(std::move(next));
        consume_type(lex, Token::SemiColumn);
    }
    consume_name(lex, "block");
    consume_type(lex, Token::Digit);
    if(lex.number != index) 
        report_error(lex, lex.cur, "Expected end of the block: " + std::to_string(index));
    consume_type(lex, Token::Column);
    consume_name(lex, "end");
}

StepResult Block::execute() {
    for(auto &step : steps) {
        StepResult res = step->execute();
        if(res.res != StepResult::Value::Continue)
            return res;
    }
    return StepResult();
}
