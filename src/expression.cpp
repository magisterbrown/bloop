#include <string>
#include <vector>

#include "bloop.h"

class Node : public SExpr {
public:
    enum class Operator { Add, Multiply, More, Less, Eq };
    Node(std::unique_ptr<SExpr> left, std::unique_ptr<SExpr> right, Operator operate) : left(std::move(left)), right(std::move(right)), operate(operate) {}
    int get() override { 
        switch(operate) {
            case Operator::Add:      return left->get() + right->get(); 
            case Operator::Multiply: return left->get() * right->get(); 
            case Operator::More:     return left->get() > right->get(); 
            case Operator::Less:     return left->get() < right->get(); 
            case Operator::Eq:       return left->get() == right->get(); 
        }
        UNREACHABLE("Unsupported operator in expression");
    };
private:
    std::unique_ptr<SExpr> left;
    std::unique_ptr<SExpr> right;
    Operator operate;
};

class ProcedureDigit: public SExpr {
public:
    ProcedureDigit(std::shared_ptr<Procedure> proc, Lexer &lex, std::shared_ptr<Context> context, ParsingContext parsc) : proc(proc) {
        consume_type(lex, Token::LSquareBracket);
        for(;;) {
            args.push_back(parse_expression(lex, context, parsc));
            if(peek_next(lex) == Token::RSquareBracket)
                break;
            consume_type(lex, Token::Comma);        
        }
        consume_type(lex, Token::RSquareBracket);
        if(args.size() != proc->context->parameters.size()) 
            report_error(lex, lex.cur, "Procedure: " + proc->name + " expected " + std::to_string(proc->context->parameters.size()) + " argument but received " + std::to_string(args.size()) + " arguments.");
    };
    int get() override {
        std::vector<int> evaluated_args;
        for(auto &arg : args)
            evaluated_args.push_back(arg->get());
        return proc->execute(evaluated_args);
        return 1;
    };
private:
    std::shared_ptr<Procedure> proc;
    std::vector<std::unique_ptr<SExpr>> args;
};

class OutputDigit : public SExpr {
public:
    OutputDigit(std::shared_ptr<Context> context) : context(context) {};
    int get() override {return context->output;};
private:
    std::shared_ptr<Context> context;
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
        case Token::Multiply: return 3;
        case Token::Plus: return 2;
        case Token::More: return 1;
        case Token::Less: return 1;
        case Token::Eq: return 1;
        case Token::LBracket: return -1;
        default: UNREACHABLE("Recieved non expression operator in expression");
    }
}

int process_op(std::vector<std::unique_ptr<SExpr>> &operands, std::vector<Token> &operators) {
    if(operands.size()<2 || operators.empty())
        return 1;
    auto right = std::move(operands.back());  operands.pop_back();
    auto left  = std::move(operands.back());  operands.pop_back();
    auto op    = std::move(operators.back()); operators.pop_back();
    switch(op) {
        case Token::Multiply: operands.push_back(std::make_unique<Node>(std::move(left), std::move(right), Node::Operator::Multiply)); break;
        case Token::Plus: operands.push_back(std::make_unique<Node>(std::move(left), std::move(right), Node::Operator::Add)); break;
        case Token::More: operands.push_back(std::make_unique<Node>(std::move(left), std::move(right), Node::Operator::More)); break;
        case Token::Less: operands.push_back(std::make_unique<Node>(std::move(left), std::move(right), Node::Operator::Less)); break;
        case Token::Eq: operands.push_back(std::make_unique<Node>(std::move(left), std::move(right), Node::Operator::Eq)); break;
        default: UNREACHABLE("Recieved non expression operator in expression");
    }
   return 0; 
}

std::unique_ptr<SExpr> parse_expression(Lexer &lex, std::shared_ptr<Context> context, ParsingContext &parsc) {
    std::vector<std::unique_ptr<SExpr>> operands;
    std::vector<Token> operators;
    Cur back;
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
                    report_error(lex, lex.cur, "cell(" + std::to_string(cell_index) + ") have not been initialized");
                }
                operands.push_back(std::make_unique<CellDigit>(context, cell_index));
            } else if(lex.string.compare("output") == 0) {
                operands.push_back(std::make_unique<OutputDigit>(context));
            } else if(context->parameters.find(lex.string) != context->parameters.end()) {
                operands.push_back(std::make_unique<ParamDigit>(context, lex.string));
            } else if(parsc.defined.find(lex.string) != parsc.defined.end()) {
                operands.push_back(std::make_unique<ProcedureDigit>(parsc.defined[lex.string], lex, context, parsc));
            } else break;

        } else if(next == Token::RBracket) {
            while(operators.back() != Token::LBracket) {
                if(process_op(operands, operators) != 0)
                    report_error(lex, lex.cur, "Can not parse an expression"); 
            }
            operators.pop_back();

        } else if(next == Token::Plus || next == Token::Multiply || next == Token::More || next == Token::Less || next == Token::Eq) {
            while(!operators.empty() && get_prio(operators.back()) >= get_prio(next))
                if(process_op(operands, operators) != 0)
                    report_error(lex, lex.cur, "Can not parse an expression"); 
            operators.push_back(next);

        } else {
            break;
        }
    }
    while(!operators.empty()) 
        if(process_op(operands, operators) != 0)
            report_error(lex, lex.cur, "Can not parse an expression"); 
    if(operands.size()!=1) {
        report_error(lex, lex.cur, "Can not parse an expression"); 
    }
    lex.cur = back;
    return std::move(operands[0]);
}
