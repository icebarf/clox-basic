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

struct Expr_t {
    enum EXPR_TYPES { LITERAL, UNARY, BINARY, GROUPING } type;
    union Holder {
        struct Binary_e {
            Expr* left;
            Token* Operator;
            Expr* right;
            void (*accept)(struct Binary_e*);
        } Binary_e;

        struct Grouping_e {
            Expr* expression;
            void (*accept)(struct Grouping_e*);
        } Grouping_e;

        struct Unary_e {
            Token* Operator;
            Expr* right;
            void (*accept)(struct Unary_e*);
        } Unary_e;

        struct Literal_e {
            enum TOKEN_TYPE type;
            union LITERAL_VALUE {
                double number;
                char* string;
                bool boolean;
            } value;
            void (*accept)(struct Literal_e*);
        } Literal_e;

        /* set this acceptor to sub-expression's acceptor is
         * don't know where it will be useful, but I will keep
         * it here. Unless I find it is useless, it will not be removed
         */
        void (*accept)(void*);
    } hld;
};

#endif