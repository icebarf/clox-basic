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

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "ast_printer.h"
#include "environment.h"
#include "evaluator.h"
#include "parser.h"
#include "program.h"
#include "token.h"
#include "utility.h"

enum { STMT_CNT = 30 };

/***** parser utility functions *****/

struct Binary_e
init_binary_expr(Expr* left,
                 Token Operator,
                 Expr* right,
                 void (*visitor)(Env_manager* env_mgr, struct Binary_e*))
{
    return (struct Binary_e){ .left = left,
                              .Operator = Operator,
                              .right = right,
                              .accept = visitor,
                              .nests = false };
}

struct Grouping_e
init_group_expr(Expr* expression,
                void (*visitor)(Env_manager* env_mgr, struct Grouping_e*))
{
    return (struct Grouping_e){ .expression = expression, .accept = visitor };
}

struct Unary_e
init_unary_expr(Token Operator,
                Expr* right,
                void (*visitor)(Env_manager* env_mgr, struct Unary_e*))
{
    return (
      struct Unary_e){ .Operator = Operator, .right = right, .accept = visitor };
}

struct Literal_e
init_literal_expr(Token token,
                  void (*visitor)(Env_manager* env_mgr, struct Literal_e*))
{
    return (struct Literal_e){ .value = token, .accept = visitor };
}

struct Variable_e
init_variable_expr(Token name,
                   Expr* rvalue,
                   void (*visitor)(Env_manager* env_mgr, struct Variable_e*))
{
    return (struct Variable_e){ .name = name, .value = rvalue, .accept = visitor };
}

Expr
init_expression(enum EXPR_TYPES type,
                void* holder,
                void (*visitor)(Expr*),
                Object (*evaluator)(Env_manager* env_mgr, Expr*, bool*))
{
    Expr expr = { .type = type, .accept = visitor, .evaluate = evaluator };
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
    return (Parser){ .tokens = tokens };
}

static inline void
MEM_LOG(enum EXPR_TYPES cond)
{
    UNUSED(cond);
#ifdef CLOX_LOG_ALLOCATIONS
    switch (cond) {
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
        case VARIABLE:
            puts("Deallocating Variable RHS expression");
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
            struct Grouping_e* g = expr->group;
            deallocate_expr(g->expression);
            MEM_LOG_DEALLOC(struct Grouping_e, g)
            MEM_LOG(GROUPING);
            free(expr);
        } break;
        case BINARY: {
            struct Binary_e* b = expr->binary;
            deallocate_expr(b->left);
            deallocate_expr(b->right);
            MEM_LOG_DEALLOC(struct Binary_e, b)
            MEM_LOG(BINARY);
            free(expr);
        } break;
        case UNARY: {
            struct Unary_e* u = expr->unary;
            deallocate_expr(u->right);
            MEM_LOG_DEALLOC(struct Unary_e, u)
            MEM_LOG(UNARY);
            free(expr);
        } break;
        case LITERAL: {
            MEM_LOG_DEALLOC(struct Literal_e, expr->literal);
            MEM_LOG(LITERAL);
            free(expr);
        } break;
        case VARIABLE: {
            deallocate_expr(expr->variable->value);
            MEM_LOG_DEALLOC(struct Variable_e, expr->variable);
            MEM_LOG(VARIABLE);
            free(expr);
            break;
        }
        case INVALID_EXPR_INT: {
            MEM_LOG(INVALID_EXPR_INT);
            free(expr);
        }
    }
}

/***** Parser workers *****/

/* forward declaration */
Expr*
expression_rule(Parser* parser);

Token
peek_token(Parser* parser)
{
    return parser->tokens[parser->current_token_idx];
}

Token
previous_token(Parser* parser)
{
    return parser->tokens[parser->current_token_idx - 1];
}

static void
REPORT_PARSER_ERROR_INTERNAL(Token token, const char* message)
{
    if (token.type == ENDOF) {
        const char* fmt = "at end %s";
        size_t len = snprintf(NULL, 0, fmt, message);
        char* buffer = malloc(len + 1);
        snprintf(buffer, len + 1, fmt, message);

        error(token.line, token.col, buffer);
        free(buffer);
    } else {
        const char* fmt = "at '%s' %s";
        size_t len = snprintf(NULL, 0, fmt, token.lexeme, message);
        char* buffer = malloc(len + 1);
        snprintf(buffer, len + 1, fmt, token.lexeme, message);

        error(token.line, token.col, buffer);
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
    if (!parser_is_at_end(parser)) parser->current_token_idx++;
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

    /* if we don't match the expected token just report parser error */
    parser_error(peek_token(parser), message);
    advance_parser(parser);
    return (Token){ .type = INVALID_TOKEN_INT };
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
        *literal = init_literal_expr(token, NULL);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, NULL, &evaluate);
        return expr;
    }

    if (match_token(parser, 1, TRUE)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Literal_e);
        *literal = init_literal_expr(token, NULL);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, NULL, &evaluate);
        return expr;
    }

    if (match_token(parser, 1, NIL)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Literal_e);
        *literal = init_literal_expr(token, NULL);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, NULL, &evaluate);
        return expr;
    }

    if (match_token(parser, 1, STRING)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Literal_e);
        *literal = init_literal_expr(token, NULL);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, NULL, &evaluate);
        return expr;
    }

    if (match_token(parser, 1, NUMBER)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Literal_e);
        *literal = init_literal_expr(token, NULL);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, NULL, &evaluate);
        return expr;
    }

    if (match_token(parser, 1, IDENTIFIER)) {
        token = previous_token(parser);

        literal = MEM_LOG_ALLOC(struct Variable_e);
        *literal = init_literal_expr(token, NULL);

        expr = MEM_LOG_ALLOC(Expr);
        *expr = init_expression(LITERAL, literal, NULL, &evaluate);
        return expr;
    }

    if (match_token(parser, 1, LEFT_PAREN)) {
        expr = expression_rule(parser);

        consume(parser, RIGHT_PAREN, "Expected a ')' after expression.");

        struct Grouping_e* grp = MEM_LOG_ALLOC(struct Grouping_e);
        *grp = init_group_expr(expr, NULL);

        Expr* gexpr = MEM_LOG_ALLOC(Expr);
        *gexpr = init_expression(GROUPING, grp, NULL, &evaluate);

        return gexpr;
    }

    /* if nothing matches then it is just bad */
    expr = MEM_LOG_ALLOC(Expr);
    *expr = init_expression(INVALID_EXPR_INT, NULL, NULL, &evaluate);
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
        *unary = init_unary_expr(Operator, right, NULL);

        Expr* unary_expr = MEM_LOG_ALLOC(Expr);
        *unary_expr = init_expression(UNARY, unary, NULL, &evaluate);
        return unary_expr;
    }

    return primary_rule(parser);
}

Expr*
factor_rule(Parser* parser)
{
    Expr* binary_expr = unary_rule(parser);
    size_t cnt = 0;

    while (match_token(parser, 3, SLASH, MOD, STAR)) {
        Token Operator = previous_token(parser);
        Expr* right = unary_rule(parser);
        if (binary_expr->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on LHS");
        }
        if (right->type == INVALID_EXPR_INT) {
            parser_error(Operator, "Expected an operand on RHS");
        }

        struct Binary_e* binary = MEM_LOG_ALLOC(struct Binary_e);
        *binary = init_binary_expr(binary_expr, Operator, right, NULL);
        if (cnt) binary->nests = true;

        binary_expr = MEM_LOG_ALLOC(Expr);
        *binary_expr = init_expression(BINARY, binary, NULL, &evaluate);
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
        *binary = init_binary_expr(binary_expr, Operator, right, NULL);
        if (cnt) binary->nests = true;

        binary_expr = MEM_LOG_ALLOC(Expr);
        *binary_expr = init_expression(BINARY, binary, NULL, &evaluate);
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
        *binary = init_binary_expr(binary_expr, Operator, right, NULL);
        if (cnt) binary->nests = true;

        binary_expr = MEM_LOG_ALLOC(Expr);
        *binary_expr = init_expression(BINARY, binary, NULL, &evaluate);
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
        *binary = init_binary_expr(binary_expr, Operator, right, NULL);
        if (cnt) binary->nests = true;

        binary_expr = MEM_LOG_ALLOC(Expr);
        *binary_expr = init_expression(BINARY, binary, NULL, &evaluate);
        cnt++;
    }

    return binary_expr;
}

Expr*
assignment_rule(Parser* parser)
{
    Expr* expr = equality_rule(parser);

    if (match_token(parser, 1, EQUAL)) {
        Token equals = previous_token(parser);
        Expr* rvalue = assignment_rule(parser);

        if (expr->type == VARIABLE) {
            Token name = expr->variable->name;
            deallocate_expr(expr);

            struct Variable_e* assign = MEM_LOG_ALLOC(struct Variable_e);
            *assign = init_variable_expr(name, rvalue, NULL);

            Expr* assigned = MEM_LOG_ALLOC(Expr);
            *assigned = init_expression(VARIABLE, assign, NULL, &evaluate);

            return assigned;
        }

        REPORT_PARSER_ERROR_INTERNAL(equals, "Invalid lvalue for assignment.");
    }

    return expr;
}

Expr*
expression_rule(Parser* parser)
{
    return assignment_rule(parser);
}

void*
allocate_statements(size_t count)
{
    return malloc(count * sizeof(Statement));
}

static Statement
print_statement(Parser* parser, Env_manager* env_mgr)
{
    Expr* value = expression_rule(parser);
    Token semicolon = consume(parser, SEMICOLON, "Expected a ';' after expression.");
    if (value->type == INVALID_EXPR_INT) parser->had_error = true;
    if (semicolon.type == INVALID_TOKEN_INT) {
        deallocate_expr(value);
        parser->had_error = true;
    }

    return (Statement){ .type = PRINT_STMT,
                        .accept = &eval_print_stmt,
                        .prtStmt = (Print_statement){ .expression = value,
                                                      .semicolon = semicolon },
                        .env_idx = env_mgr->env_idx };
}

static Statement
expression_statement(Parser* parser, Env_manager* env_mgr)
{
    Expr* value = expression_rule(parser);
    Token semicolon = consume(parser, SEMICOLON, "Expected a ';' after expression.");
    if (value->type == INVALID_EXPR_INT) parser->had_error = true;
    if (semicolon.type == INVALID_TOKEN_INT) {
        deallocate_expr(value);
        parser->had_error = true;
    }

    return (Statement){ .type = EXPR_STMT,
                        .accept = &eval_expr_stmt,
                        .exStmt = (Expr_statement){ .expression = value,
                                                    .semicolon = semicolon },
                        .env_idx = env_mgr->env_idx };
}

static Statement
var_declaration(Parser* parser, Env_manager* env_mgr)
{
    Token name = consume(parser, IDENTIFIER, "Expected identifier.");
    Expr* init = NULL;

    if (match_token(parser, 1, EQUAL)) {
        init = expression_rule(parser);
        if (init->type == INVALID_EXPR_INT) parser->had_error = true;
    }

    Token semicolon = consume(parser, SEMICOLON, "Expected a ';' after expression.");
    if (semicolon.type == INVALID_TOKEN_INT) {
        deallocate_expr(init);
        parser->had_error = true;
    }

    return (Statement){ .type = VAR_DECL_STMT,
                        .vardecl = (Var_decl){ .name = name, .initialiser = init },
                        .accept = &eval_var_stmt,
                        .env_idx = env_mgr->env_idx };
}

static Statement
declaration(Parser* parser, Env_manager* env_mgr);

static Statement
block(Parser* parser, Env_manager* env_mgr)
{
    Statement* statements = allocate_statements(STMT_CNT * sizeof(Statement));
    size_t idx = 0;
    size_t cnt = 0;
    size_t have_stmts = 0;

    env_mgr->envs =
      realloc(env_mgr->envs, (sizeof(Environment*) * 1) + env_mgr->env_idx);

    sh_new_arena(env_mgr->envs[env_mgr->env_idx]);
    env_mgr->env_idx++;

    while (!check_token(parser, RIGHT_BRACE) && !parser_is_at_end(parser)) {
        if (cnt == have_stmts) {
            statements = realloc(statements, have_stmts * 2 * sizeof(Statement));
            if (statements == NULL) {
                REPORT_PARSER_ERROR_INTERNAL((Token){ .type = INVALID_TOKEN_INT },
                                             "Out of memory");
                exit(EX_OSERR);
            }
            have_stmts *= 2;
        }
        statements[idx] = declaration(parser, env_mgr);
        statements[idx].env_idx = env_mgr->env_idx;
        cnt++;
    }

    statements[0].count = idx - 1;

    consume(parser, RIGHT_BRACE, "Expected a '}' after block.");

    return (Statement){ .block = (Block){ .statements = statements },
                        .type = BLOCK_STMT,
                        .accept = eval_block,
                        .env_idx = env_mgr->env_idx };
}

static Statement
statement(Parser* parser, Env_manager* env_mgr)
{
    if (match_token(parser, 1, PRINT)) return print_statement(parser, env_mgr);
    if (match_token(parser, 1, LEFT_BRACE)) return block(parser, env_mgr);

    return expression_statement(parser, env_mgr);
}

static Statement
declaration(Parser* parser, Env_manager* env_mgr)
{
    if (match_token(parser, 1, VAR)) return var_declaration(parser, env_mgr);

    return statement(parser, env_mgr);
}

Statement*
parse(Program* program)
{
    program->parser->statements = allocate_statements(STMT_CNT * sizeof(Statement));
    size_t cnt = 0;
    size_t have_stmts = STMT_CNT;

    while (!parser_is_at_end(program->parser)) {
        if (cnt == have_stmts) {
            program->parser->statements = realloc(
              program->parser->statements, have_stmts * 2 * sizeof(Statement));
            if (program->parser->statements == NULL) {
                REPORT_PARSER_ERROR_INTERNAL((Token){ .type = INVALID_TOKEN_INT },
                                             "Out of memory");
                exit(EX_OSERR);
            }
            have_stmts *= 2;
        }

        program->parser->statements[program->parser->current_statement_idx++] =
          declaration(program->parser, program->env_mgr);

        if (program->parser->had_error) {
            syncronize_parser(program->parser);
            free(program->parser->statements);
            return NULL;
        }
        cnt++;
    }
    program->parser->statements[0].count = program->parser->current_statement_idx;

    return program->parser->statements;
}
