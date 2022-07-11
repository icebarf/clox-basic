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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "token.h"

/* global TokenTypeString array for different Token Types*/
const char* TokenTypeString[] = {
    /* Single Character Tokens */
    [LEFT_PAREN] = "left_paren",
    [RIGHT_PAREN] = "right_paren",
    [LEFT_BRACE] = "left_brace",
    [RIGHT_BRACE] = "right_brace",
    [COMMA] = "comma",
    [DOT] = "dot",
    [MINUS] = "minus",
    [PLUS] = "plus",
    [SEMICOLON] = "semicolon",
    [SLASH] = "slash",
    [STAR] = "star",
    [MOD] = "mod",

    /* One or Two character Tokens*/
    [BANG] = "bang",
    [BANG_EQUAL] = "bang_equal",
    [EQUAL] = "equal",
    [EQUAL_EQUAL] = "equal_equal",
    [GREATER] = "greater",
    [GREATER_EQUAL] = "greater_equal",
    [LESS] = "less",
    [LESS_EQUAL] = "less_equal",

    /* Literals */
    [IDENTIFIER] = "identifier",
    [STRING] = "string",
    [NUMBER] = "number",

    /* Keywords */
    [AND] = "and",
    [CLASS] = "class",
    [ELSE] = "else",
    [FALSE] = "false",
    [FUN] = "fun",
    [FOR] = "for",
    [IF] = "if",
    [NIL] = "nil",
    [OR] = "or",
    [PRINT] = "print",
    [RET] = "ret",
    [SUPER] = "super",
    [THIS] = "this",
    [TRUE] = "true",
    [VAR] = "var",
    [WHILE] = "while",

    [ENDOF] = "endof",
};

/* initialise a token by calling this function
 * Params:
 * @type : the type of token scanned
 * @lexeme : the lexeme/word scanned
 * @num : a double precision floating point literal scanned
 * @line : the line where the lexeme was found
 */
Token
init_tok(const enum TOKEN_TYPE type,
         const char* lexeme,
         const double num,
         const int line)
{
    Token token = {
        .type = type,
        .line = line,
    };
    if (type == NUMBER) {
        token.num_literal = num;
        return token;
    }
    token.lexeme = (char*)lexeme;
    return token;
}

/* returns a human readabble null-terminated formatted string from Token
 * Params:
 * @token : the token which is to be converted
 * Ret:
 * @const char* : the formatted string
 */
const char*
token_to_str(const Token* token)
{
    char* fmt = "%15s '%s' ";
    size_t fmt_len =
      snprintf(NULL, 0, fmt, TokenTypeString[token->type], token->lexeme);

    if (token->type == NUMBER) {
        fmt = "%15s %lf ";
        fmt_len =
          snprintf(NULL, 0, fmt, TokenTypeString[token->type], token->num_literal);
    }
    char* buf = malloc(fmt_len + 1);

    if (token->type == NUMBER) {
        snprintf(
          buf, fmt_len, fmt, TokenTypeString[token->type], token->num_literal);
        return buf;
    }

    snprintf(buf, fmt_len, fmt, TokenTypeString[token->type], token->lexeme);
    return buf;
}

/* allocate count amount of tokens
 * Params:
 * @count : number of tokens to be allocated initially
 * Ret:
 * @void* : a block of memory obtained from malloc
 */
void*
allocate_tokens(unsigned long count)
{
    return malloc(sizeof(Token) * count);
}

/* extend previously allocated tokens by prev_count + count amount of tokens
 * Params:
 * @tokens : the previously allocated Tokens
 * @prev_count : number of tokens allocated previously
 * @count : number of tokens to extend by
 * Ret:
 * @void* : extended block of memory obtained from realloc
 */
void
extend_tokens_by(Token* tokens, unsigned long prev_count, unsigned long count)
{
    tokens = realloc(tokens, (prev_count + count) * sizeof(Token));
}

/* deallocate the tokens - free the lexemes and then the tokens block
 * Params:
 * @tokens : main block of tokens
 * @tokencnt : the number of tokens present in 'tokens'*/
void
deallocate_tokens(Token* tokens, size_t tokencnt)
{
    size_t counter = 0;

    /* last EndOfFile Token does not need free()-ing*/
    while (counter < tokencnt) {
        if (tokens[counter].type == NUMBER) {
            counter++;
            continue;
        }
        free(tokens[counter].lexeme);
        counter++;
    }
    free(tokens);
}