#include <fstream>
#include <iostream>
#include <sstream>

#include "lexer.h"

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
    Lexer lex = Lexer(source_code);
    lex.next_token();
    do {
        std::cout << "Token: " << lex.print_token() << std::endl;
        lex.next_token();
    } while(lex.tok!=Token::Eof);
}
