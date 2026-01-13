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
    Block(Lexer &lex, std::shared_ptr<Context> context);
    void execute();
private: 
    std::vector<std::unique_ptr<Step>> steps;
    int index;
};
#endif
