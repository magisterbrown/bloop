enum class Token
{
    Identifier,
    Backticks,
    LSquareBracket,
    RSquareBracket,
    LBracket,
    RBracket,
    Assign,
    Multiply,
    Column,
    Eof,
    None,
    Digit,
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
