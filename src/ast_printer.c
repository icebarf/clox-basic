#include <stdio.h>

#include "ast_printer.h"
#include "parser.h"

void literal_to_str(struct Literal_e* literal)
{
    switch (literal->type) {
        case NUMBER:
            fprintf(stdout, "%lf ", literal->value.number);
            break;
        case STRING:
            fprintf(stdout, "%s ", literal->value.string);
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

void unary_to_str(struct Unary_e* unary)
{
    fprintf(stdout, "%c ", unary->Operator->lexeme[0]);
    print_expr(unary->right);
}

void binary_to_str(struct Binary_e* binary)
{
    fprintf(stdout, "%c ", binary->Operator->lexeme[0]);
    print_expr(binary->left);
    print_expr(binary->right);
}

void grouping_to_str(struct Grouping_e* grp)
{
    fprintf(stdout, "( ");
    print_expr(grp->expression);
    fprintf(stdout, " )");
}

void print_expr(Expr* ex)
{
    switch (ex->type) {
        case LITERAL:
            literal_to_str(&ex->hld.Literal_e);
            break;
        case UNARY:
            unary_to_str(&ex->hld.Unary_e);
            break;
        case BINARY:
            binary_to_str(&ex->hld.Binary_e);
            break;
        case GROUPING:
            grouping_to_str(&ex->hld.Grouping_e);
        default:
            break;
    }
}