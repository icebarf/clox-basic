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

#include "ast_printer.h"
#include "parser.h"
#include "token.h"
#include "utility.h"

static inline void
ast_error(void)
{
    fprintf(stderr, RED_2 "lox_internal: argument to ast printer is null\n" RESET);
}

void
literal_to_str(struct Literal_e* literal)
{
    if (literal == NULL) {
        ast_error();
        return;
    }

    switch (literal->value.type) {
        case NUMBER:
            fprintf(stdout, "%lf ", literal->value.num_literal);
            break;
        case STRING:
            fprintf(stdout, "%s ", literal->value.lexeme);
            break;
        case TRUE:
            fprintf(stdout, "%s ", literal->value.lexeme);
            break;
        case FALSE:
            fprintf(stdout, "%s ", literal->value.lexeme);
            break;
        case NIL:
            fprintf(stdout, "%s ", literal->value.lexeme);
            break;
        default:
            break;
    }
}

void
unary_to_str(struct Unary_e* unary)
{
    if (unary == NULL) {
        ast_error();
        return;
    }
    fprintf(stdout, "%s ", unary->Operator.lexeme);
    unary->right->accept(unary->right);
}

void
binary_to_str(struct Binary_e* binary)
{

    if (binary == NULL) {
        ast_error();
        return;
    }
    fprintf(stdout, "%s ", binary->Operator.lexeme);
    if (binary->nests) fprintf(stdout, "(");
    binary->left->accept(binary->left);
    if (binary->nests) fprintf(stdout, ") ");
    binary->right->accept(binary->right);
}

void
grouping_to_str(struct Grouping_e* grp)
{
    if (grp == NULL) {
        ast_error();
        return;
    }
    fprintf(stdout, "(");
    grp->expression->accept(grp->expression);
    fprintf(stdout, ") ");
}

void
print_expr(Expr* ex)
{
    if (ex == NULL) {
        ast_error();
        return;
    }
    switch (ex->type) {
        case LITERAL: {
            struct Literal_e* l = ex->expression_structure;
            if (l->accept == NULL) {
                ast_error();
                break;
            }
            l->accept(l);
        } break;
        case UNARY: {
            struct Unary_e* u = ex->expression_structure;
            if (u->accept == NULL) {
                ast_error();
                break;
            }
            u->accept(u);
        } break;
        case BINARY: {
            struct Binary_e* b = ex->expression_structure;
            if (b->accept == NULL) {
                ast_error();
                break;
            }
            b->accept(b);
        } break;
        case GROUPING: {
            struct Grouping_e* g = ex->expression_structure;
            if (g->accept == NULL) {
                ast_error();
                break;
            }
            g->accept(g);
        } break;
        default:
            break;
    }
}