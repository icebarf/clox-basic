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

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "evaluator.h"
#include "parser.h"
#include "program.h"
#include "token.h"
#include "utility.h"
#include "environment.h"

enum LIMITS { DOUBLE_MAX_DIG = 19 };

/**** utility functions for the evaluator ****/

Object
evaluate_identifier(Env_manager* env_mgr, Expr* expr);

static Object
get_object_from_literal(Env_manager* env_mgr, Expr* expr)
{
    if (expr->type != LITERAL) return (Object){ .type = INVALID_TOKEN_INT };

    switch (expr->literal->value.type) {
        case NUMBER:
            return (Object){ .number = expr->literal->value.num_literal,
                             .string = expr->literal->value.lexeme,
                             .string_len = expr->literal->value.lexeme_len,
                             .type = NUMBER };
        case STRING:
            return (Object){ .string = expr->literal->value.lexeme,
                             .string_len = expr->literal->value.lexeme_len,
                             .type = STRING };
        case TRUE:
            return (Object){ .string = expr->literal->value.lexeme,
                             .string_len = expr->literal->value.lexeme_len,
                             .type = TRUE };
        case FALSE:
            return (Object){ .string = expr->literal->value.lexeme,
                             .string_len = expr->literal->value.lexeme_len,
                             .type = FALSE };
        case NIL:
            return (Object){ .string = expr->literal->value.lexeme, .type = NIL };

        case IDENTIFIER:
            return evaluate_identifier(env_mgr, expr);

        default:
            return (Object){ .type = INVALID_TOKEN_INT };
    }
}

static bool
is_bool(Object object)
{
    return object.type == TRUE || object.type == FALSE;
}

static bool
is_truthy(Object object)
{
    if (object.type == INVALID_TOKEN_INT) return false;
    if (object.type == NIL) return false;

    if (is_bool(object)) {
        return is_bool(object);
    }

    return true;
}

static enum TOKEN_TYPE
boolean_type(bool b)
{
    if (b == true) return TRUE;
    if (b == false) return FALSE;

    __builtin_unreachable();
}

static bool
is_floating_almost_equal(double a, double b)
{
    double diff = 0;
    double sum = 0;
    double EPSILON = 0.000001;

    diff = fabs(a - b);
    sum = fabs(a + b);

    if (diff <= (2 * EPSILON * sum)) return true;
    return false;
}

static bool
True(Object a)
{
    return a.type == TRUE;
}

static bool
False(Object a)
{
    return a.type == FALSE;
}

static bool
is_equal(Object a, Object b)
{
    if (a.type == INVALID_TOKEN_INT && b.type == INVALID_TOKEN_INT) return true;
    if (a.type == INVALID_TOKEN_INT || b.type == INVALID_TOKEN_INT) return false;
    if (a.type == NIL && b.type == NIL) return true;
    if (a.type == NIL || b.type == NIL) return false;

    /* boolean truth table */
    if (True(a) && True(b)) return true;
    if (True(a) && False(b)) return false;
    if (False(a) && True(b)) return false;
    if (False(a) && False(b)) return true;

    /* comparison between a string and a number is always false */
    if ((a.type == STRING && b.type == NUMBER) ||
        (a.type == NUMBER && b.type == STRING)) {
        return false;
    }

    /* comparison between a number and a boolean is always false */
    if (((a.type == TRUE || a.type == FALSE) && b.type == NUMBER) ||
        (a.type == NUMBER && (b.type == TRUE || b.type == FALSE))) {
        return false;
    }

    /* compare two strings */
    if (a.type == STRING && b.type == STRING) {
        int z = strcmp(a.string, b.string);
        if (z == 0) return true;
        return false;
    }

    /* compare two numbers */
    return is_floating_almost_equal(a.number, b.number);
}

static bool
check_number_operands(enum TOKEN_TYPE chktype, size_t objcnt, ...)
{
    Object obj;
    va_list obj_list;
    va_start(obj_list, objcnt);

    for (size_t i = 0; i < objcnt; i++) {
        obj = va_arg(obj_list, Object);
        if (obj.type != chktype) return false;
    }
    return true;
}

static void
runtime_error(Token Operator, const char* message, bool* had_runtime_error)
{
    error(Operator.line, Operator.col, message);
    *had_runtime_error = true;
}

/****** Actual Evaluator code ******/
static Object
evaluate_literal(Env_manager* env_mgr, Expr* expr)
{
    return get_object_from_literal(env_mgr, expr);
}

static Object
evaluate_unary(Env_manager* env_mgr, Expr* expr, bool* had_runtime_error)
{
    Object right = evaluate(env_mgr, expr->unary->right, had_runtime_error);

    switch (expr->unary->Operator.type) {
        case MINUS:
            if (!check_number_operands(NUMBER, 1, right)) {
                runtime_error(expr->unary->Operator,
                              "Runtime: Operand must be a number",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            return (Object){ .number = -right.number, .type = NUMBER };

        case BANG: {
            /* only invalid object and false and NIL are Falsy, rest are Truthy
             */
            bool what = !is_truthy(right);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        default:
            __builtin_unreachable();
    }
}

static Object
evaluate_group(Env_manager* env_mgr, Expr* expr, bool* had_runtime_error)
{
    return evaluate(env_mgr, expr->group->expression, had_runtime_error);
}

static Object
evaluate_binary(Env_manager* env_mgr, Expr* expr, bool* had_runtime_error)
{
    Object left = evaluate(env_mgr, expr->binary->left, had_runtime_error);
    Object right = evaluate(env_mgr, expr->binary->right, had_runtime_error);

    switch (expr->binary->Operator.type) {
        case MINUS:
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            return (Object){ .number = left.number - right.number, .type = NUMBER };

        case PLUS:
            if ((left.type == NUMBER) && (right.type == NUMBER)) {
                return (Object){ .number = left.number + right.number,
                                 .type = NUMBER };
            }

            if (((left.type == NUMBER) &&
                 ((right.type == STRING) || (right.type == STRING_2))) ||
                (((left.type == STRING) || (left.type == STRING_2)) &&
                 (right.type == NUMBER)) ||
                (((left.type == STRING) || (left.type == STRING_2)) &&
                 ((right.type == STRING) || (right.type == STRING_2)))) {
                char* bigstr =
                  calloc(left.string_len + right.string_len + 1, sizeof(char));

                memccpy(bigstr, left.string, '\0', left.string_len);
                strncat(bigstr, right.string, right.string_len);

                if (left.type == STRING_2) free(left.string);
                else if (right.type == STRING_2)
                    free(right.string);
                return (Object){ .string = bigstr,
                                 .string_len =
                                   left.string_len + right.string_len + 1,
                                 .type = STRING_2 };
            }

            runtime_error(expr->binary->Operator,
                          "Runtime: Operands must either be a number or a string.",
                          had_runtime_error);
            return (Object){ .type = INVALID_TOKEN_INT };

        case SLASH:
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            if (is_floating_almost_equal(right.number, 0.0f)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Division by zero is not allowed.",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            return (Object){ .number = left.number / right.number, .type = NUMBER };

        case MOD:
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            if (is_floating_almost_equal(right.number, 0.0f)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Division by zero is not allowed.",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            return (Object){ .number = fmod(left.number, right.number),
                             .type = NUMBER };

        case STAR:
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            return (Object){ .number = left.number * right.number, .type = NUMBER };

        case GREATER: {
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            bool what = isgreater(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case GREATER_EQUAL: {
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            bool what = isgreaterequal(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case LESS: {
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            bool what = isless(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case LESS_EQUAL: {
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers",
                              had_runtime_error);
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            bool what = islessequal(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case BANG_EQUAL: {
            bool what = !is_equal(left, right);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case EQUAL_EQUAL: {
            bool what = is_equal(left, right);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        default:
            return (Object){ .type = INVALID_TOKEN_INT };
    }

    __builtin_unreachable();
    return (Object){ .type = INVALID_TOKEN_INT };
}

Object
evaluate_identifier(Env_manager* env_mgr, Expr* expr)
{
    return get_value(env_mgr, expr->literal->value, env_mgr->env_idx);
}

static Object
evaluate_assignment(Env_manager* env_mgr, Expr* expr, bool* had_runtime_error)
{
    Object value = evaluate(env_mgr, expr->variable->value, had_runtime_error);
    if (value.type != INVALID_TOKEN_INT)
        assign(env_mgr, expr->variable->name, value, env_mgr->env_idx);
    else {
        value = (Object){ .string = expr->variable->name.lexeme,
                          .string_len = expr->variable->name.lexeme_len,
                          .type = IDENTIFIER };
    }
    return value;
}

Object
evaluate(Env_manager* env_mgr, Expr* expr, bool* had_runtime_error)
{
    if (expr == NULL) return (Object){ .type = INVALID_TOKEN_INT };
    switch (expr->type) {
        case LITERAL:
            return evaluate_literal(env_mgr, expr);
        case UNARY:
            return evaluate_unary(env_mgr, expr, had_runtime_error);
        case GROUPING:
            return evaluate_group(env_mgr, expr, had_runtime_error);
        case BINARY:
            return evaluate_binary(env_mgr, expr, had_runtime_error);
        case VARIABLE:
            return evaluate_assignment(env_mgr, expr, had_runtime_error);
        default:
            __builtin_unreachable();
    }
}

static char*
stringify(Object object)
{
    if (object.type == NIL) return "nil";

    if (object.type == NUMBER) {
        char* str = calloc(DOUBLE_MAX_DIG + 1, sizeof(char));
        snprintf(str, DOUBLE_MAX_DIG, "%lf", object.number);
        return str;
    }

    if (object.type == STRING || object.type == STRING_2) {
        return object.string;
    }

    if (object.type == TRUE) {
        return "true";
    }

    if (object.type == FALSE) {
        return "false";
    }

    __builtin_unreachable();
    return NULL;
}

void
eval_expr_stmt(Env_manager* env_mgr, Statement statement, bool* had_runtime_error)
{
    evaluate(env_mgr, statement.exStmt.expression, had_runtime_error);
}

void
eval_print_stmt(Env_manager* env_mgr, Statement statement, bool* had_runtime_error)
{
    char* str = NULL;
    Object obj = evaluate(env_mgr, statement.prtStmt.expression, had_runtime_error);
    if (obj.type == INVALID_TOKEN_INT) return;

    if (obj.type == IDENTIFIER)
        obj = evaluate_identifier(env_mgr, statement.prtStmt.expression);

    str = stringify(obj);
    puts(str);
    if (obj.type == NUMBER || (obj.type == STRING_2)) free(str);
}

void
eval_var_stmt(Env_manager* env_mgr, Statement statement, bool* had_runtime_error)
{
    Object obj = { .type = NIL };
    if (statement.vardecl.expression != NULL)
        obj = evaluate(env_mgr, statement.vardecl.expression, had_runtime_error);

    define(env_mgr, statement.vardecl.tok.lexeme, obj, env_mgr->env_idx);
}

void
eval_block(Env_manager* env_mgr, Statement statement, bool* had_runtime_error)
{
    Statement* block = statement.block.statements;
    size_t cnt = block[0].count;

    for (size_t i = 0; i < cnt; i++) {
        block[i].accept(env_mgr, block[i], had_runtime_error);
    }

    shfree(env_mgr->envs[env_mgr->env_idx]);
    env_mgr->env_idx--;
    for (size_t i = 0; i < cnt; i++)
        deallocate_expr(block[i].exStmt.expression);
    free(block);
}

void
interpret(Program* program)
{
    for (size_t i = 0; i < program->statements[0].count; i++) {
        program->statements[i].accept(
          program->env_mgr, program->statements[i], &program->had_runtime_error);
    }
}
