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

/**** utility functions for the evaluator ****/

Object
get_object_from_literal(Expr* expr)
{
    if (expr->type != LITERAL) return (Object){ .type = INVALID_TOKEN_INT };

    switch (expr->literal->value.type) {
        case NUMBER:
            return (Object){ .number = expr->literal->value.num_literal,
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

bool
is_bool(Object object)
{
    return object.type == TRUE || object.type == FALSE;
}

bool
is_truthy(Object object)
{
    if (object.type == INVALID_TOKEN_INT) return false;
    if (object.type == NIL) return false;

    if (is_bool(object)) {
        return is_bool(object);
    }

    return true;
}

enum TOKEN_TYPE
boolean_type(bool b)
{
    if (b == true) return TRUE;
    if (b == false) return FALSE;

    __builtin_unreachable();
}

bool
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

bool
True(Object a)
{
    return a.type == TRUE;
}

bool
False(Object a)
{
    return a.type == FALSE;
}

bool
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

bool
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

void
runtime_error(Token Operator, const char* message)
{
    error(Operator.line, message);
    had_runtime_error = true;
}

/****** Actual Evaluator code ******/
Object
evaluate_literal(Expr* expr)
{
    return get_object_from_literal(expr);
}

Object
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
            return (Object){ .number = -right.number,
                             .type = expr->unary->Operator.type };

        case BANG: {
            /* only invalid object and false and NIL are Falsy, rest are Truthy */
            bool what = !is_truthy(right);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        default:
            __builtin_unreachable();
    }
}

Object
evaluate_group(Expr* expr)
{
    return evaluate(expr->group->expression);
}

Object
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
            if ((left.type == STRING) && (right.type == STRING)) {
                char* bigstr =
                  calloc(left.string_len + right.string_len + 1, sizeof(char));

                memccpy(bigstr, left.string, left.string_len, sizeof(char));
                strcat(bigstr, right.string);
                return (Object){ .string = bigstr, .type = STRING };
            }

            runtime_error(expr->binary->Operator,
                          "Runtime: Operands must be two numbers or two strings.");
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

char*
stringify(Object object)
{
    if (object.type == NIL) return "nil";

    if (object.type == NUMBER) {
        char* str = calloc(1, sizeof(double) + 1);
        sprintf(str, "%lf", object.number);
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

void
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