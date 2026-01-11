#include <map>
#include <string>

#ifndef CONTEXT_H
#define CONTEXT_H

class Context {
public: 
    std::map<int, int> cells;
    std::map<std::string, int> parameters;
    int output = 0;
};

#endif
