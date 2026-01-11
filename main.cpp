#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <memory>

#include "lexer.h"

class Context {
public: 
    std::map<int, int> cells;
    std::map<std::string, int> parameters;
    int output = 0;
};

Token get_next(Lexer &lex) {
    bool success = lex.next_token();
    if(!success) {
        // TODO: raise parsing error   
        abort();
    }
    return lex.tok;
}

Token peek_next(Lexer &lex) {
    Cur save = lex.cur;
    Token res = get_next(lex);
    lex.cur = save;
    return res;
}
void consume_type(Lexer &lex, Token expected) {
    Token actual = get_next(lex);
    if(actual != expected) {
        abort();
    }
}

void consume_name(Lexer &lex, std::string name) {
    consume_type(lex, Token::Identifier);
    if(lex.string.compare(name) != 0) {
        abort();
    }
}

class Step {
public: 
    void execute();
};

class SExpr {
public:
    virtual int get() = 0;
    virtual ~SExpr() = default; 
};

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
    CellDigit(Context &context, int cell_idx) : context(context), cell_idx(cell_idx) {};
    int get() override {return context.cells[cell_idx];};
private:
    Context &context;
    int cell_idx;
};

class ParamDigit : public SExpr {
public:
    ParamDigit(Context &context, std::string param_name) : context(context), param_name(param_name) {};
    int get() override {return context.parameters[param_name];};
private:
    Context &context;
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
std::unique_ptr<SExpr> parse_expression(Lexer &lex, Context &context) {
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
                if(context.cells.find(cell_index) == context.cells.end()){
                    // Uninitialized cell
                    abort();
                }
                operands.push_back(std::make_unique<CellDigit>(context, cell_index));
            } else if(context.parameters.find(lex.string) != context.parameters.end()) {
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
    asm("int3");
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
