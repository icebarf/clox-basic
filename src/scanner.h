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

#ifndef CLOX_BASIC_SCANNER_H
#define CLOX_BASIC_SCANNER_H

#include "token.h"
#include <stddef.h>

#define TOKEN_CNT 120L

typedef struct Scanner {
    const char* source;
    Token* tokens;
    size_t token_max;
    size_t tokens_count;
    size_t source_length;
    size_t start;
    size_t current;
    size_t line;
} Scanner;

/* initialise the scanner */
Scanner init_scanner(const char* source, const size_t source_length);

/* start scanning the tokens */
Token* scan_tokens(Scanner* scanner);

#endif