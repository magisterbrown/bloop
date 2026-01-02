#include <print>
#include <fstream>

enum TokenType
{
    Identifier,
    Backticks,
    LSquareBracket,
    RSquareBracket,
    Column,
}

int main() {
    std::ifstream input("to_the_n.bloop");
    char ch;
    while(input.get(ch)) {
        std::println("{}", ch); 
    }
}
