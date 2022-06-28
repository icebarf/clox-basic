// clox-basic - C Language Implementation of jlox from Crafting Interpreters.
//
// Copyright (C) 2022 Amritpal Singh
//
// This file is part of clox-basic.
//
// clox-basic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3
// of the License, or (at your option) any later version.
//
// clox-basic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// clox-basic. If not, see <https://www.gnu.org/licenses/>.

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