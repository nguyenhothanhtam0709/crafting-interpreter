#ifndef LOX_TOKEN_HH
#define LOX_TOKEN_HH

#include <string>

enum class TokenType
{
    // ===== Single-character tokens =====
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,
    QUESTION_MARK,
    COLON,

    // ===== One or two character tokens =====
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    // ===== Literals =====
    IDENTIFIER,
    STRING,
    NUMBER,

    // ===== Keywords =====
    AND,
    CLASS,
    ELSE,
    FALSE,
    FUN,
    FOR,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,
    BREAK,
    CONTINUE,

    END_OF_LINE
};

class Token
{
public:
    Token(TokenType type, const std::string &lexeme, int line);

    const TokenType type;
    const std::string lexeme;
    const int line;
};

#endif