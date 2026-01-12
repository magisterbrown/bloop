#include <vector>
#include "context.h"
#include "lexer.h"

#ifndef BLOCK_H
#define BLOCK_H

class Step {
public: 
    virtual void execute() = 0;
    virtual ~Step() = default;
};

class Block {
public: 
    Block() = default;
    Block(Lexer &lex, Context &context);
    void execute();
private: 
    //std::vector<Step> steps;
    int index;
};
#endif
