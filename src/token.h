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

#include <stddef.h>
enum TOKEN_TYPE {
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

    ENDOF,
    // invalid token value - meant for internal use
    INVALID_TOKEN_INT
};

typedef struct Token {
    char* lexeme;
    double num_literal;
    size_t lexeme_len;
    size_t line;
    size_t col;
    enum TOKEN_TYPE type;
} Token;

/* initialise token with value */
Token
init_tok(enum TOKEN_TYPE type,
         char* lexeme,
         size_t lexeme_len,
         double num,
         size_t line,
         size_t col);

/* convert token to string representation */
const char*
token_to_str(const Token* token);

/* will allocate space for 'count' amount of tokens */
void*
allocate_tokens(const unsigned long count);

/* extend the memory allocated for tokens by    'prev_count + count'
 * Note: 'count' is the amount of new tokens to be added not sum ^ */
void
extend_tokens_by(Token* tokens,
                 const unsigned long prev_count,
                 const unsigned long count);

/* free() the allocated memory for the tokens */
void
deallocate_tokens(Token* tokens, size_t tokencnt);

#endif