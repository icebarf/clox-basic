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

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include "ast_printer.h"
#include "parser.h"
#include "token.h"
#include "utility.h"

extern bool had_error;
/***** parser utility functions *****/

struct Binary_e
init_binary_expr(Expr* left,
                 Token Operator,
                 Expr* right,
                 void (*visitor)(struct Binary_e*))
{
    return (struct Binary_e){ .left = left,
                              .Operator = Operator,
                              .right = right,
                              .accept = visitor,
                              .nests = false };
}

struct Grouping_e
init_group_expr(Expr* expression, void (*visitor)(struct Grouping_e*))
{
    return (struct Grouping_e){ .expression = expression, .accept = visitor };
}

struct Unary_e
init_unary_expr(Token Operator, Expr* right, void (*visitor)(struct Unary_e*))
{
    return (
      struct Unary_e){ .Operator = Operator, .right = right, .accept = visitor };
}

struct Literal_e
init_literal_expression(Token token, void (*visitor)(struct Literal_e*))
{
    return (struct Literal_e){ .value = token, .accept = visitor };
}

Expr
init_expression(enum EXPR_TYPES type, void* holder, void (*visitor)(Expr*))
{
    Expr expr = { .type = type, .accept = visitor };
    switch (type) {
        case LITERAL:
            expr.literal = holder;
            return expr;
        case UNARY:
            expr.unary = holder;
            return expr;
        case BINARY:
            expr.binary = holder;
            return expr;
        case GROUPING:
            expr.group = holder;
            return expr;
        default:
            return expr;
    }
}

Parser
init_parser(Token* tokens)
{
    return (Parser){ .tokens = tokens, .current = 0 };
}

static inline void
MEM_LOG(enum EXPR_TYPES cond)
{
    (void)cond;
#ifdef CLOX_LOG_ALLOCATIONS
    switch (cond) {
        case ALLOC:
            puts("Allocating Expression");
            break;
        case GROUPING:
            puts("Deallocating Group Expression");
            break;
        case BINARY:
            puts("Deallocating Binary Expression");
            break;
        case UNARY:
            puts("Deallocating Unary Experssion");
            break;
        case LITERAL:
            puts("Deallocating Literal Expression");
            break;
        case INVALID_EXPR_INT:
            puts("Deallocating Invalid Expression");
            break;
    }
#endif
}

/* custom allocator - deallocator */
#define MEM_LOG_ALLOC(T) malloc(sizeof(T))

#ifdef CLOX_LOG_ALLOCATIONS
#undef MEM_LOG_ALLOC
#define MEM_LOG_ALLOC(T)                                                            \
    malloc(sizeof(T));                                                              \
    puts("Allocating " #T)
#endif

#define MEM_LOG_DEALLOC(T, x) free(x);

#ifdef CLOX_LOG_ALLOCATIONS
#undef MEM_LOG_DEALLOC
#define MEM_LOG_DEALLOC(T, x)                                                       \
    puts("Deallocating " #T);                                                       \
    free(x);
#endif

void
deallocate_expr(Expr* expr)
{
    switch (expr->type) {
        case GROUPING: {
            MEM_LOG(GROUPING);
            struct Grouping_e* g = expr->group;
            deallocate_expr(g->expression);
            MEM_LOG_DEALLOC(struct Grouping_e, g)
            free(expr);
        } break;
        case BINARY: {
            MEM_LOG(BINARY);
            struct Binary_e* b = expr->binary;
            deallocate_expr(b->left);
            deallocate_expr(b->right);
            MEM_LOG_DEALLOC(struct Binary_e, b)
            free(expr);
        } break;
        case UNARY: {
            MEM_LOG(UNARY);
            struct Unary_e* u = expr->unary;
            deallocate_expr(u->right);
            MEM_LOG_DEALLOC(struct Unary_e, u)
            free(expr);
        } break;
        case LITERAL: {
            MEM_LOG(LITERAL);
            MEM_LOG_DEALLOC(struct Literal_e, expr->literal);
            free(expr);
        } break;
        case INVALID_EXPR_INT: {
            MEM_LOG(INVALID_EXPR_INT);
            free(expr);
        }
        default:
            return;
    }
}

/***** Parser workers *****/

/* forward declaration */
Expr*
expression_rule(Parser* parser);

Token
peek_token(Parser* parser)
{
    return parser->tokens[parser->current];
}

Token
previous_token(Parser* parser)
{
    return parser->tokens[parser->current - 1];
}

static void
REPORT_PARSER_ERROR_INTERNAL(Token token, const char* message)
{
    if (token.type == ENDOF) {
        const char* fmt = "at end %s";
        size_t len = snprintf(NULL, 0, fmt, message);
        char* buffer = malloc(len + 1);
        snprintf(buffer, len + 1, fmt, message);

        error(token.line, buffer);
        free(buffer);
    } else {
        const char* fmt = "at '%s' %s";
        size_t len = snprintf(NULL, 0, fmt, token.lexeme, message);
        char* buffer = malloc(len + 1);
        snprintf(buffer, len + 1, fmt, token.lexeme, message);

        error(token.line, buffer);
        free(buffer);
    }
}

void
parser_error(Token token, const char* message)
{
    REPORT_PARSER_ERROR_INTERNAL(token, message);
}

bool
parser_is_at_end(Parser* parser)
{
    Token tok = peek_token(parser);
    return tok.type == ENDOF;
}

Token
advance_parser(Parser* parser)
{
    if (!parser_is_at_end(parser)) parser->current++;
    return previous_token(parser);
}

bool
check_token(Parser* parser, enum TOKEN_TYPE type)
{
    if (parser_is_at_end(parser)) return false;
    Token tok = peek_token(parser);
    return tok.type == type;
}

bool
match_token(Parser* parser, size_t token_type_cnt, ...)
{
    enum TOKEN_TYPE type;
    va_list type_list;
    va_start(type_list, token_type_cnt);

    for (size_t i = 0; i < token_type_cnt; i++) {
        type = va_arg(type_list, enum TOKEN_TYPE);
        if (check_token(parser, type)) {
            advance_parser(parser);
            return true;
        }
    }
    return false;
}

void
syncronize_parser(Parser* parser)
{
    advance_parser(parser);
    while (!parser_is_at_end(parser)) {
        Token tok = previous_token(parser);
        if (tok.type == SEMICOLON) return;

        tok = peek_token(parser);

        switch (tok.type) {
            case CLASS:
            case FUN:
            case VAR:
            case FOR:
            case IF:
            case WHILE:
            case PRINT:
            case RET:
                return;
            default:
                break;
        }

        advance_parser(parser);
    }
}

Token
consume(Parser* parser, enum TOKEN_TYPE type, const char* message)
{
    if (check_token(parser, type)) return advance_parser(parser);

    /* if we don't match the expected token just set errno and report parser error */
    errno = EIO;
    parser_error(peek_token(parser), message);
    return (Token){ .type = INVALID_TOKEN_INT,
                    .lexeme = NULL,
                    .num_literal = 0,
                    .line = 0,
                    .col = 0 };
}

Expr*
primary_rule(Parser* parser)
{
    Expr* expr = NULL;
    struct Literal_e* literal = NULL;
    Token token = { 0 };

    if (match_token(parser, 1, FALSE)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Literal_e);
        *literal = init_literal_expression(token, &literal_to_str);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, &print_expr);
        return expr;
    }

    if (match_token(parser, 1, TRUE)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Literal_e);
        *literal = init_literal_expression(token, &literal_to_str);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, &print_expr);
        return expr;
    }

    if (match_token(parser, 1, NIL)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Literal_e);
        *literal = init_literal_expression(token, &literal_to_str);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, &print_expr);
        return expr;
    }

    if (match_token(parser, 1, STRING)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Literal_e);
        *literal = init_literal_expression(token, &literal_to_str);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, &print_expr);
        return expr;
    }

    if (match_token(parser, 1, NUMBER)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Literal_e);
        *literal = init_literal_expression(token, &literal_to_str);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, &print_expr);
        return expr;
    }

    if (match_token(parser, 1, LEFT_PAREN)) {
        expr = expression_rule(parser);

        consume(parser, RIGHT_PAREN, "Expect a ')' after expression.");

        struct Grouping_e* grp = MEM_LOG_ALLOC(struct Grouping_e);
        *grp = init_group_expr(expr, &grouping_to_str);

        Expr* gexpr = MEM_LOG_ALLOC(Expr);
        *gexpr = init_expression(GROUPING, grp, &print_expr);

        return gexpr;
    }

    /* if nothing matches then it is just bad */
    expr = MEM_LOG_ALLOC(Expr);
    *expr = init_expression(INVALID_EXPR_INT, NULL, NULL);
    return expr;
}

Expr*
unary_rule(Parser* parser)
{
    if (match_token(parser, 2, BANG, MINUS)) {
        Token Operator = previous_token(parser);
        Expr* right = unary_rule(parser);
        if (right->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Invalid operand on RHS");
            return right;
        }

        struct Unary_e* unary = MEM_LOG_ALLOC(struct Unary_e);
        *unary = init_unary_expr(Operator, right, &unary_to_str);

        Expr* unary_expr = MEM_LOG_ALLOC(Expr);
        *unary_expr = init_expression(UNARY, unary, &print_expr);
        return unary_expr;
    }

    return primary_rule(parser);
}

Expr*
factor_rule(Parser* parser)
{
    Expr* binary_expr = unary_rule(parser);
    size_t cnt = 0;

    while (match_token(parser, 2, SLASH, STAR)) {
        Token Operator = previous_token(parser);
        Expr* right = unary_rule(parser);
        if (binary_expr->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on LHS");
        }
        if (right->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on RHS");
        }

        struct Binary_e* binary = MEM_LOG_ALLOC(struct Binary_e);
        *binary = init_binary_expr(binary_expr, Operator, right, &binary_to_str);
        if (cnt) binary->nests = true;

        binary_expr = MEM_LOG_ALLOC(Expr);
        *binary_expr = init_expression(BINARY, binary, &print_expr);
        cnt++;
    }

    return binary_expr;
}

Expr*
term_rule(Parser* parser)
{
    Expr* binary_expr = factor_rule(parser);
    size_t cnt = 0;

    while (match_token(parser, 2, MINUS, PLUS)) {
        Token Operator = previous_token(parser);
        Expr* right = factor_rule(parser);
        if (binary_expr->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on LHS");
        }
        if (right->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on RHS");
        }

        struct Binary_e* binary = MEM_LOG_ALLOC(struct Binary_e);
        *binary = init_binary_expr(binary_expr, Operator, right, &binary_to_str);
        if (cnt) binary->nests = true;

        binary_expr = MEM_LOG_ALLOC(Expr);
        *binary_expr = init_expression(BINARY, binary, print_expr);
        cnt++;
    }

    return binary_expr;
}

Expr*
comparison_rule(Parser* parser)
{
    Expr* binary_expr = term_rule(parser);
    size_t cnt = 0;

    while (match_token(parser, 4, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
        Token Operator = previous_token(parser);
        Expr* right = term_rule(parser);
        if (binary_expr->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on LHS");
        }
        if (right->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on RHS");
        }

        struct Binary_e* binary = MEM_LOG_ALLOC(struct Binary_e);
        *binary = init_binary_expr(binary_expr, Operator, right, &binary_to_str);
        if (cnt) binary->nests = true;

        binary_expr = MEM_LOG_ALLOC(Expr);
        *binary_expr = init_expression(BINARY, binary, &print_expr);
        cnt++;
    }

    return binary_expr;
}

Expr*
equality_rule(Parser* parser)
{
    Expr* binary_expr = comparison_rule(parser);
    size_t cnt = 0;

    while (match_token(parser, 2, BANG_EQUAL, EQUAL_EQUAL)) {
        Token Operator = previous_token(parser);
        Expr* right = comparison_rule(parser);
        if (binary_expr->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on LHS");
        }
        if (right->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on RHS");
        }

        struct Binary_e* binary = MEM_LOG_ALLOC(struct Binary_e);
        *binary = init_binary_expr(binary_expr, Operator, right, &binary_to_str);
        if (cnt) binary->nests = true;

        binary_expr = MEM_LOG_ALLOC(Expr);
        *binary_expr = init_expression(BINARY, binary, &print_expr);
        cnt++;
    }

    return binary_expr;
}

Expr*
expression_rule(Parser* parser)
{
    return equality_rule(parser);
}

Expr*
parse(Parser* parser)
{
    Expr* exp = expression_rule(parser);
    if (exp == NULL) {
        had_error = true;
        return NULL;
    }
    if (exp->type == INVALID_EXPR_INT) had_error = true;
    return exp;
}