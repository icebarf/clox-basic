#include <stdio.h>
#include <stdlib.h>
#include "token.h"

const char* TokenTypeString[] = {
    /* Single Character Tokens */
    [LEFT_PAREN] = "LEFT_PAREN",
    [RIGHT_PAREN] = "RIGHT_PAREN",
    [LEFT_BRACE] = "LEFT_BRACE",
    [RIGHT_BRACE] = "RIGHT_BRACE",
    [COMMA] = "COMMA",
    [DOT] = "DOT",
    [MINUS] = "MINUS",
    [PLUS] = "PLUS",
    [SEMICOLON] = "SEMICOLON",
    [SLASH] = "SLASH",
    [STAR] = "STAR",
    [MOD] = "MOD",

    /* One or Two character Tokens*/
    [BANG] = "BANG",
    [BANG_EQUAL] = "BANG_EQUAL",
    [EQUAL] = "EQUAL",
    [EQUAL_EQUAL] = "EQUAL_EQUAL",
    [GREATER] = "GREATER",
    [GREATER_EQUAL] = "GREATER_EQUAL",
    [LESS] = "LESS",
    [LESS_EQUAL] = "LESS_EQUAL",

    /* Literals */
    [IDENTIFIER] = "IDENTIFIER",
    [STRING] = "STRING",
    [NUMBER] = "NUMBER",

    /* Keywords */
    [AND] = "AND",
    [CLASS] = "CLASS",
    [ELSE] = "ELSE",
    [FALSE] = "FALSE",
    [FUN] = "FUN",
    [FOR] = "FOR",
    [IF] = "IF",
    [NIL] = "NIL",
    [OR] = "OR",
    [PRINT] = "PRINT",
    [RET] = "RET",
    [SUPER] = "SUPER",
    [THIS] = "THIS",
    [TRUE] = "TRUE",
    [VAR] = "VAR",
    [WHILE] = "WHILE",

    [ENDOF] = "ENDOF",
};

Token init_tok(enum TokenType type, char* lexeme, char* literal, int line)
{
    Token token = {
        .type = type, .lexeme = lexeme, .literal = literal, .line = line};
    return token;
}

const char* token_to_str(const Token token)
{
    const char* fmt = "%s %s %s";
    size_t fmt_len = snprintf(NULL, 0, fmt, TokenTypeString[token.type],
                              token.lexeme, token.literal);
    char* buf = malloc(fmt_len + 1);

    snprintf(buf, fmt_len, TokenTypeString[token.type], token.lexeme,
             token.literal);

    return buf;
}