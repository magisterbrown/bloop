#include <stdexcept>
#include <string>
#include "lexer.h"

class ExecutionError : public std::exception {
public: 
    std::string msg;
    Cur point
    ExecutionError(std::string msg, Cur point) : msg(msg), point(point) {}
}
