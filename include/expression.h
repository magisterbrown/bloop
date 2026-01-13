#include <memory>
#include "lexer.h"

#ifndef EXPR_H
#define EXPR_H

class SExpr {
public: virtual int get() = 0;
    virtual ~SExpr() = default; 
};

std::unique_ptr<SExpr> parse_expression(Lexer &lex, std::shared_ptr<Context> context); 

#endif
