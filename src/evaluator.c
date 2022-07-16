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
#include "token.h"
#include "utility.h"

extern bool had_runtime_error;
enum LIMITS { DOUBLE_MAX_DIG = 19 };

/**** utility functions for the evaluator ****/

static Object
get_object_from_literal(Expr* expr)
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
runtime_error(Token Operator, const char* message)
{
    error(Operator.line, Operator.col, message);
    had_runtime_error = true;
}

/****** Actual Evaluator code ******/
static Object
evaluate_literal(Expr* expr)
{
    return get_object_from_literal(expr);
}

static Object
evaluate_unary(Expr* expr)
{
    Object right = evaluate(expr->unary->right);

    switch (expr->unary->Operator.type) {
        case MINUS:
            if (!check_number_operands(NUMBER, 1, right)) {
                runtime_error(expr->unary->Operator,
                              "Runtime: Operand must be a number");
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
evaluate_group(Expr* expr)
{
    return evaluate(expr->group->expression);
}

static Object
evaluate_binary(Expr* expr)
{
    Object left = evaluate(expr->binary->left);
    Object right = evaluate(expr->binary->right);

    switch (expr->binary->Operator.type) {
        case MINUS:
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers");
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            return (Object){ .number = left.number - right.number, .type = NUMBER };

        case PLUS:
            if ((left.type == NUMBER) && (right.type == NUMBER)) {
                return (Object){ .number = left.number + right.number,
                                 .type = NUMBER };
            }

            if (((left.type == NUMBER) && (right.type == STRING)) ||
                ((left.type == STRING) && (right.type == NUMBER)) ||
                ((left.type == STRING) && (right.type == STRING))) {

                char* bigstr =
                  calloc(left.string_len + right.string_len + 1, sizeof(char));

                memccpy(bigstr, left.string, '\0', left.string_len);
                strncat(bigstr, right.string, right.string_len);
                return (Object){ .string = bigstr, .type = STRING };
            }

            runtime_error(expr->binary->Operator,
                          "Runtime: Operands must either be a number or a string.");
            return (Object){ .type = INVALID_TOKEN_INT };

        case SLASH:
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers");
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            if (is_floating_almost_equal(right.number, 0.0f)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Division by zero is not allowed.");
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            return (Object){ .number = left.number / right.number, .type = NUMBER };

        case MOD:
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers");
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            if (is_floating_almost_equal(right.number, 0.0f)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Division by zero is not allowed.");
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            return (Object){ .number = fmod(left.number, right.number),
                             .type = NUMBER };

        case STAR:
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers");
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            return (Object){ .number = left.number * right.number, .type = NUMBER };

        case GREATER: {
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers");
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            bool what = isgreater(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case GREATER_EQUAL: {
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers");
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            bool what = isgreaterequal(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case LESS: {
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers");
                return (Object){ .type = INVALID_TOKEN_INT };
            }
            bool what = isless(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case LESS_EQUAL: {
            if (!check_number_operands(NUMBER, 2, left, right)) {
                runtime_error(expr->binary->Operator,
                              "Runtime: Operands must be numbers");
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
evaluate(Expr* expr)
{
    switch (expr->type) {
        case LITERAL:
            return evaluate_literal(expr);
        case UNARY:
            return evaluate_unary(expr);
        case GROUPING:
            return evaluate_group(expr);
        case BINARY:
            return evaluate_binary(expr);
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

    if (object.type == STRING) {
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

static void
deallocate_object(Object o)
{
    if (o.type == STRING) free(o.string);
}

void
interpret(Expr* expr)
{
    Object obj = expr->evaluate(expr);
    if (obj.type == INVALID_TOKEN_INT) {
        return;
    }
    char* str = stringify(obj);
    fprintf(stdout, "%s\n", str);
    if (obj.type == NUMBER) free(str);

    deallocate_object(obj);
}