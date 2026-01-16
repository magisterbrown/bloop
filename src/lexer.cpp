#include <string>

#include "lexer.h"

#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)


std::string print_token(Token tok) {
    switch(tok) {
        case Token::Identifier:     return "Identifier";
        case Token::Backticks:      return "Backticks";
        case Token::LSquareBracket: return "LSquareBracket";
        case Token::RSquareBracket: return "RSquareBracket";
        case Token::Column:         return "Column";
        case Token::SemiColumn:     return "SemiColumn";
        case Token::Eof:            return "Eof";
        case Token::Digit:          return "Digit";
        case Token::LBracket:       return "LBracket";
        case Token::RBracket:       return "RBracket";
        case Token::Assign:         return "Assign";
        case Token::Multiply:       return "Multiply";
        case Token::Plus:           return "Plus";
        case Token::Dot:            return "Dot";
        case Token::More:           return "More";
        case Token::Less:           return "Less";
        case Token::Eq:             return "Eq";
        case Token::None:           return "None";
    }
    return "";
}
char Lexer::get_char() {
    if(cur.pos>=_content.size()) return 0;
    return _content[cur.pos];
}
char Lexer::next_char() {
    char res = _content[cur.pos++];
    if(res == '\n') {
        cur.row++;
        cur.bol = cur.pos+1;
    }
    return res;
}
Lexer::Lexer(std::string content) : _content(content), tok(Token::None) {}

bool Lexer::next_token() {
    while(std::isspace(get_char()))
        next_char();            

    char ch = next_char();
    if(ch == '\0') {
        tok=Token::Eof; return true;
    }

    if(ch == '`') {
        ch = next_char();
        if(ch != '`')
            return false;
        tok=Token::Backticks; return true;
    }

    if(ch == '<' && get_char() == '=') {
        tok=Token::Assign; return true;
    }

    switch(ch) {
        case '[': tok=Token::LSquareBracket; return true;
        case ']': tok=Token::RSquareBracket; return true;
        case ':': tok=Token::Column; return true;
        case ';': tok=Token::SemiColumn; return true;
        case '*': tok=Token::Multiply; return true;
        case '+': tok=Token::Plus; return true;
        case '(': tok=Token::LBracket; return true;
        case ')': tok=Token::RBracket; return true;
        case ',': tok=Token::Comma; return true;
        case '.': tok=Token::Dot; return true;
        case '>': tok=Token::More; return true;
        case '<': tok=Token::Less; return true;
        case '=': tok=Token::Less; return true;
    }

    if(std::isalpha(ch)) {
        string.clear();
        string.push_back(ch);
        ch = get_char();
        while(std::isalnum(ch) || ch  == '-' || ch =='?') {
            string.push_back(next_char());
            ch = get_char();
        }
        tok=Token::Identifier; return true;
    }
    if(std::isdigit(ch)) {
        number = ch - '0'; 
        ch = get_char();
        while(std::isdigit(ch)) {
            number*=10;
            number+=next_char() - '0';
            ch = get_char();
        }
        tok=Token::Digit; return true;
    }
    return false;
}
