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

#include "parser.h"
#include "token.h"

struct Binary_e
init_binary_expr(Expr* left,
                 Token* Operator,
                 Expr* right,
                 void (*visitor)(struct Binary_e*))
{
    return (struct Binary_e){
        .left = left, .Operator = Operator, .right = right, .accept = visitor
    };
}

struct Grouping_e
init_group_expr(Expr* expression, void (*visitor)(struct Grouping_e*))
{
    return (struct Grouping_e){ .expression = expression, .accept = visitor };
}

struct Unary_e
init_unary_expr(Token* Operator, Expr* right, void (*visitor)(struct Unary_e*))
{
    return (
      struct Unary_e){ .Operator = Operator, .right = right, .accept = visitor };
}

struct Literal_e
init_literal_expression(enum TOKEN_TYPE type,
                        union LITERAL_VALUE literal,
                        void (*visitor)(struct Literal_e*))
{
    return (struct Literal_e){ .type = type, .value = literal, .accept = visitor };
}