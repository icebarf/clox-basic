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

#ifndef CLOX_BASIC_AST_PRINTER_H
#define CLOX_BASIC_AST_PRINTER_H

#include "parser.h"
#include "token.h"

/* prints a literal expression to stdout */
void
literal_to_str(struct Literal_e* literal);

/* prints a unary expression to stdout */
void
unary_to_str(struct Unary_e* unary);

/* prints a binary expression to stdout */
void
binary_to_str(struct Binary_e* binary);

/* prints a group expression to stdout */
void
grouping_to_str(struct Grouping_e* grp);

/* print an expression */
void
print_expr(Env_manager*, Expr*);
#endif