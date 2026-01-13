#include <string>
#include <vector>

#include "expression.h"
#include "context.h"
#include "parsing.h"

class Node : public SExpr {
public:
    enum class Operator { Add, Multiply };
    Node(std::unique_ptr<SExpr> left, std::unique_ptr<SExpr> right, Operator operate) : left(std::move(left)), right(std::move(right)), operate(operate) {}
    int get() override { 
        switch(operate) {
            case Operator::Add:     return left->get() + right->get(); 
            case Operator::Multiply: return left->get() * right->get(); 
        }
        abort();
    };
private:
    std::unique_ptr<SExpr> left;
    std::unique_ptr<SExpr> right;
    Operator operate;
};
    
class CellDigit : public SExpr {
public:
    CellDigit(std::shared_ptr<Context> context, int cell_idx) : context(context), cell_idx(cell_idx) {};
    int get() override {return context->cells[cell_idx];};
private:
    std::shared_ptr<Context> context;
    int cell_idx;
};

class ParamDigit : public SExpr {
public:
    ParamDigit(std::shared_ptr<Context> context, std::string param_name) : context(context), param_name(param_name) {};
    int get() override {return context->parameters[param_name];};
private:
    std::shared_ptr<Context> context;
    std::string param_name;
};

class Digit : public SExpr {
public:
    Digit(int value) : value(value) {};
    int value;
    int get() override {return value;};
};

int get_prio(Token op) {
    switch(op) {
        case Token::Multiply: return 2;
        case Token::Plus: return 1;
        case Token::LBracket: return -1;
        default: abort();
    }
}

void process_op(std::vector<std::unique_ptr<SExpr>> &operands, std::vector<Token> &operators) {
    if(operands.size()<2 || operators.empty())
        abort();
    auto right = std::move(operands.back());  operands.pop_back();
    auto left  = std::move(operands.back());  operands.pop_back();
    auto op    = std::move(operators.back()); operators.pop_back();
    switch(op) {
        case Token::Multiply: operands.push_back(std::make_unique<Node>(std::move(left), std::move(right), Node::Operator::Multiply)); break;
        case Token::Plus: operands.push_back(std::make_unique<Node>(std::move(left), std::move(right), Node::Operator::Add)); break;
        default: abort();
    }
    
}

std::unique_ptr<SExpr> parse_expression(Lexer &lex, std::shared_ptr<Context> context) {
    std::vector<std::unique_ptr<SExpr>> operands;
    std::vector<Token> operators;
    Cur back;
    //TODO: factor out in expression
    for(;;) {
        back = lex.cur;
        auto next = get_next(lex);
        if(next == Token::Digit) operands.push_back(std::make_unique<Digit>(lex.number));
        else if(next == Token::LBracket) operators.push_back(Token::LBracket);
        else if(next == Token::Identifier) {
            //TODO accept output as input variable
            if(lex.string.compare("cell") == 0) {
                consume_type(lex, Token::LBracket);
                consume_type(lex, Token::Digit);
                int cell_index = lex.number;
                consume_type(lex, Token::RBracket);
                if(context->cells.find(cell_index) == context->cells.end()){
                    // Uninitialized cell
                    abort();
                }
                operands.push_back(std::make_unique<CellDigit>(context, cell_index));
            } else if(context->parameters.find(lex.string) != context->parameters.end()) {
                operands.push_back(std::make_unique<ParamDigit>(context, lex.string));
            } else break;

        } else if(next == Token::RBracket) {
            while(operators.back() != Token::LBracket) {
                process_op(operands, operators);
            }
            operators.pop_back();

        } else if(next == Token::Plus || next == Token::Multiply) {
            while(!operators.empty() && get_prio(operators.back()) >= get_prio(next))
                process_op(operands, operators);
            operators.push_back(next);

        } else break;
    }
    while(!operators.empty()) 
        process_op(operands, operators);
    if(operands.size()!=1) {
        abort();
    }
    lex.cur = back;
    return std::move(operands[0]);
}
