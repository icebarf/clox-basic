#ifndef CLOX_BASIC_TOKEN_H
#define CLOX_BASIC_TOKEN_H

enum TokenType {
    /* Single Character Tokens */
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
    MOD,

    /* One or Two character Tokens*/
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    /* Literals */
    IDENTIFIER,
    STRING,
    NUMBER,

    /* Keywords */
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
    RET,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    ENDOF
};

typedef struct Token {
    enum TokenType type;
    char* lexeme;
    void* literal;
    int line;
    int col;
} Token;

/* initialise token with value */
Token init_tok(enum TokenType type, char* lexeme, char* literal, int line);

/* convert token to string representation */
const char* token_to_str(const Token token);

#endif