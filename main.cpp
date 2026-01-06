#include <print>
#include <sstream>
#include <fstream>

enum Token
{
    Identifier,
    Backticks,
    LSquareBracket,
    RSquareBracket,
    Column,
    Eof,
};

struct Cur {
    size_t pos, row;
    Cur() : pos(0), row(0) {}
};

class Lexer {
public:
    Lexer(std::string content);
    Token get_next();

    Cur cur;

    int number;
    std::string string;

    void print_content();
private:
    std::string _content;
    char get_char();
    char next_char();
};
char Lexer::get_char() {
    if(this->cur.pos>=_content.size()) return 0;
    return _content[cur.pos];
}
char Lexer::next_char() {
    char res = _content[cur.pos++];
    if(res == '\n')
        this->cur.row++;
    return res;
}
Lexer::Lexer(std::string content) : _content(content) {}
Token Lexer::get_next() {
    // Consume spaces
    while(std::isspace(this->get_char()))
        this->next_char();            

    char ch = this->next_char();
    if(ch == '\0') 
        return Eof;

    if(ch == '`') {
        ch = this->next_char();
        if(ch != '`')
            abort();
        return Backticks;
    }

    switch(ch) {
        case '[': return LSquareBracket;
        case ']': return RSquareBracket;
        case ':': return Column;
    }

    //asm("int3");
    if(std::isalpha(ch)) {
        this->string.clear();
        this->string.push_back(ch);
        ch = this->get_char();
        while(std::isalpha(ch) || ch  == '-' || ch =='?') {
            this->string.push_back(this->next_char());
            ch = this->get_char();
        }
        return Identifier;
    }
    abort(); // TODO: Raise parsing error
}
void Lexer::print_content() {
    println("{}", this->_content);
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
    auto nid = lex.get_next();
    while(nid!=Eof) {
        //if(nid == Backticks)
        //std::println("Token: {}", nid);
        nid = lex.get_next();
    }
    std::println("haha");
}
