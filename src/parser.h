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

#ifndef CLOX_BASIC_PARSER_H
#define CLOX_BASIC_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "token.h"

typedef struct Expr_t Expr;

/* atomic structures that make up the expression structure */
struct Binary_e {
    Token Operator;
    Expr* left;
    Expr* right;
    // visitor pattern. Make the expression visit any function.
    void (*accept)(struct Binary_e*);
    bool nests;
};

struct Grouping_e {
    Expr* expression;
    void (*accept)(struct Grouping_e*);
};

struct Unary_e {
    Token Operator;
    Expr* right;
    void (*accept)(struct Unary_e*);
};

struct Literal_e {
    Token value;
    void (*accept)(struct Literal_e*);
};

/* the molecule - expression */
struct Expr_t {
    union {
        struct Binary_e* binary;
        struct Grouping_e* group;
        struct Unary_e* unary;
        struct Literal_e* literal;
    };
    void (*accept)(Expr*);
    enum EXPR_TYPES {
        ALLOC,
        LITERAL,
        UNARY,
        BINARY,
        GROUPING,
        INVALID_EXPR_INT
    } type;
};

typedef struct {
    Token* tokens;
    size_t current;
} Parser;

/******* Functions ********/

/* initialise the parser */
Parser
init_parser(Token* tokens);

/* expression allocator */
void*
allocate_expr(void);

/* expression de-allocator */
void
deallocate_expr(Expr* expr);

/* parse an expression from the given array of tokens */
Expr*
parse(Parser* parser);

#endif