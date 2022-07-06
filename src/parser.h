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

#include "token.h"

typedef struct Expr_t Expr;

struct Binary_e {
    Expr* left;
    Token* Operator;
    Expr* right;
    // visitor pattern. Make the expression visit any function.
    void (*accept)(struct Binary_e*);
};

struct Grouping_e {
    Expr* expression;
    void (*accept)(struct Grouping_e*);
};

struct Unary_e {
    Token* Operator;
    Expr* right;
    void (*accept)(struct Unary_e*);
};

struct Literal_e {
    enum TOKEN_TYPE type;
    void* value;
    void (*accept)(struct Literal_e*);
};

struct Expr_t {
    enum EXPR_TYPES { LITERAL, UNARY, BINARY, GROUPING } type;
    void* expression_structure;
    void (*accept)(Expr*);
};

/* initialises a Binary expression */
struct Binary_e
init_binary_expr(Expr* left,
                 Token* Operator,
                 Expr* right,
                 void (*visitor)(struct Binary_e*));

/* initialises a Grouping expression */
struct Grouping_e
init_group_expr(Expr* expression, void (*visitor)(struct Grouping_e*));

/* initialises an Unary expression */
struct Unary_e
init_unary_expr(Token* Operator, Expr* right, void (*visitor)(struct Unary_e*));

/* initialises a literal expression */
struct Literal_e
init_literal_expression(enum TOKEN_TYPE type,
                        void* literal,
                        void (*visitor)(struct Literal_e*));

/* initialises an expression */
Expr
init_expression(enum EXPR_TYPES type, void* holder, void (*visitor)(Expr*));

#endif