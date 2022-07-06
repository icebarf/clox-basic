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

void
literal_to_str(struct Literal_e* literal)
{
    switch (literal->type) {
        case NUMBER:
            fprintf(stdout, "%lf ", *(double*)literal->value);
            break;
        case STRING:
            fprintf(stdout, "%s ", (const char*)literal->value);
            break;
        case TRUE:
            fprintf(stdout, "true ");
            break;
        case FALSE:
            fprintf(stdout, "false ");
            break;
        default:
            break;
    }
}

void
unary_to_str(struct Unary_e* unary)
{
    fprintf(stdout, "%c ", unary->Operator->lexeme[0]);
    print_expr(unary->right);
}

void
binary_to_str(struct Binary_e* binary)
{
    fprintf(stdout, "%s ", binary->Operator->lexeme);
    print_expr(binary->left);
    print_expr(binary->right);
}

void
grouping_to_str(struct Grouping_e* grp)
{
    fprintf(stdout, "( ");
    print_expr(grp->expression);
    fprintf(stdout, " )");
}

void
print_expr(Expr* ex)
{
    switch (ex->type) {
        case LITERAL: {
            struct Literal_e* l = ex->expression_structure;
            l->accept(l);
        } break;
        case UNARY: {
            struct Unary_e* u = ex->expression_structure;
            u->accept(u);
        } break;
        case BINARY: {
            struct Binary_e* b = ex->expression_structure;
            b->accept(b);
        } break;
        case GROUPING: {
            struct Grouping_e* g = ex->expression_structure;
            g->accept(g);
        } break;
        default:
            break;
    }
}