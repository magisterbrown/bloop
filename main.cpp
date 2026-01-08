#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <memory>

#include "lexer.h"

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
    std::unique_ptr<SExpr> left;
    std::unique_ptr<SExpr> right;
    Node(std::unique_ptr<SExpr> left, std::unique_ptr<SExpr> right) : left(std::move(left)), right(std::move(right)) {}
    int get() override { return 3;};
};
    
class RefDigit : public SExpr {
public:
    RefDigit(int &value) : value(value) {};
    int &value;
    int get() override {return value;};
};

class Digit : public SExpr {
public:
    Digit(int value) : value(value) {};
    int value;
    int get() override {return value;};
};

class Context {
public: 
    std::map<int, int> cells;
    std::map<std::string, int> parameters;
};


class Assignment : public Step {
public:
    Assignment(Lexer &lex, Context context);
private:
    Context &context;
    int target_idx;
    std::unique_ptr<SExpr> expression;
};

int get_prio(Token op){
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
    operands.push_back(std::make_unique<Node>(std::move(left), std::move(right)));
}

Assignment::Assignment(Lexer &lex, Context context) : context(context) {
    consume_name(lex, "cell");
    consume_type(lex, Token::LBracket);
    consume_type(lex, Token::Digit);
    target_idx = lex.number;
    consume_type(lex, Token::RBracket);
    consume_type(lex, Token::Assign);
    std::vector<std::unique_ptr<SExpr>> operands;
    std::vector<Token> operators;
    for(;;) {
        Cur back = lex.cur;
        auto next = get_next(lex);
        switch(next) {
            case Token::Digit: operands.push_back(std::make_unique<Digit>(lex.number)); break;
            case Token::Identifier: {
                                        if(lex.string.compare("cell") == 0) {
                                            consume_type(lex, Token::LBracket);
                                            consume_type(lex, Token::Digit);
                                            int cell_index = lex.number;
                                            consume_type(lex, Token::RBracket);
                                            operands.push_back(std::make_unique<RefDigit>(context.cells[cell_index]));
                                        } else if(context.parameters.find(lex.string) != context.parameters.end()) {
                                            operands.push_back(std::make_unique<RefDigit>(context.parameters[lex.string]));
                                        } else {
                                            abort();
                                        }
                                        break;
                                    }
            case Token::LBracket: operators.push_back(Token::LBracket); break;
            case Token::RBracket: {
                                    // TODO: handle empty stack errors
                                    while(operators.back() != Token::LBracket) {
                                        process_op(operands, operators);
                                    }
                                    operators.pop_back();
                                    break;
                                }
            default: if(next == Token::Plus || next == Token::Multiply) {
                        //Execute  
                        while(!operators.empty() && get_prio(operators.back()) >= get_prio(next)) {
                            process_op(operands, operators);
                        }
                        operators.push_back(next);
                        break;
                     } else {
                        while(!operators.empty()) {
                            process_op(operands, operators);
                        }
                        if(operands.size()!=1) {
                            abort();
                        }
                        expression = std::move(operands[0]);
                        lex.cur = back;
                        return;
                     }

        }
    }
}


class Block {
public: 
    Block(Lexer &lex, Context context);
    void execute();
private: 
    std::vector<Step> steps;
    int index;
};

Block::Block(Lexer &lex, Context context) {
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
            //Assignment
        } else if(name.compare("loop") == 0) {
            //Loop
        } else if(name.compare("output") == 0) {
            //Output
        } else if(name.compare("block") == 0) {
            break;
        }
    }
    // Finish parsing a block
}


class Procedure {
public:
    Procedure(Lexer &lex);
    int prio;
    void execute();
private:
    Context context; 
};

Procedure::Procedure(Lexer &lex) {
    consume_name(lex, "define");
    consume_name(lex, "procedure");
    consume_type(lex, Token::Backticks);
    consume_type(lex, Token::Identifier);
    std::string procedure_name = lex.string;
    consume_type(lex, Token::Backticks);
    consume_type(lex, Token::LSquareBracket);
    // TMP
    context.cells[0] = 2;
    //std::map<std::string, int> parameters;
    for(int i=0;;i++) {
        consume_type(lex, Token::Identifier);
        context.parameters[lex.string] = 0;
        Token next = get_next(lex);
        if(next == Token::RSquareBracket) 
            break;
        if(next != Token::Comma)
            abort();
    } 
    consume_type(lex, Token::Column);
    auto bl = Block(lex, context);
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
        break;
    }
}
