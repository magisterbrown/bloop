#include <sstream>
#include <fstream>
#include <iostream>


enum class Token
{
    Identifier,
    Backticks,
    LSquareBracket,
    RSquareBracket,
    Column,
    Eof,
    None,
};


struct Cur {
    size_t pos, row;
    Cur() : pos(0), row(0) {}
};

class Lexer {
public:
    Lexer(std::string content);
    bool next_token();
    std::string print_token();

    Cur cur;

    Token tok;
    int number;
    std::string string;
    
private:
    std::string _content;
    char get_char();
    char next_char();
};

std::string Lexer::print_token() {
    switch(tok) {
        case Token::Identifier:     return "Identifier(" + string + ")";
        case Token::Backticks:      return "Backticks";
        case Token::LSquareBracket: return "LSquareBracket";
        case Token::RSquareBracket: return "RSquareBracket";
        case Token::Column:         return "Column";
        case Token::Eof:            return "Eof";
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
    if(res == '\n')
        cur.row++;
    return res;
}
Lexer::Lexer(std::string content) : _content(content) {}
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

    switch(ch) {
        case '[': tok=Token::LSquareBracket; return true;
        case ']': tok=Token::RSquareBracket; return true;
        case ':': tok=Token::Column; return true;
    }

    if(std::isalpha(ch)) {
        string.clear();
        string.push_back(ch);
        ch = get_char();
        while(std::isalpha(ch) || ch  == '-' || ch =='?') {
            string.push_back(next_char());
            ch = get_char();
        }
        tok=Token::Identifier; return true;
    }
    return false;
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
    Lexer lex = Lexer(source_code);
    lex.next_token();
    do {
        //if(nid == Backticks)
        std::cout << "Token: " << lex.print_token() << std::endl;
        lex.next_token();
    } while(lex.tok!=Token::Eof);
}
