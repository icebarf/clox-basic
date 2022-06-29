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

#include <stdio.h>
#include <stdlib.h>
#include "token.h"

Token init_tok(const enum TOKEN_TYPE type,
               const char* lexeme,
               const double num,
               const int line)
{
    Token token = {.type = type,
                   .lexeme = (char*)lexeme,
                   .num_literal = num,
                   .line = line};
    return token;
}

const char* token_to_str(const Token* token)
{
    const char* fmt = "%s %s";
    size_t fmt_len =
        snprintf(NULL, 0, fmt, TokenTypeString[token->type], token->lexeme);
    char* buf = malloc(fmt_len + 1);

    snprintf(buf, fmt_len, TokenTypeString[token->type], token->lexeme);

    return buf;
}

void* allocate_tokens(unsigned long count)
{
    Token* tokens = malloc(sizeof(Token) * count);
    return tokens;
}

void extend_tokens_by(Token* tokens,
                      unsigned long prev_count,
                      unsigned long count)
{
    tokens = realloc(tokens, (prev_count + count) * sizeof(Token));
}

void deallocate_tokens(Token* tokens)
{
    free(tokens);
}