#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "evaluator.h"
#include "parser.h"
#include "token.h"
#include "utility.h"

Object
get_object_from_literal(Expr* expr)
{
    if (expr->type != LITERAL) return (Object){ .type = INVALID_TOKEN_INT };

    switch (expr->literal->value.type) {
        case NUMBER:
            return (Object){ .number = expr->literal->value.num_literal,
                             .type = NUMBER };
        case STRING:
            return (Object){ .string = expr->literal->value.lexeme, .type = STRING };
        case TRUE:
            return (Object){ .string = expr->literal->value.lexeme, .type = TRUE };
        case FALSE:
            return (Object){ .string = expr->literal->value.lexeme, .type = FALSE };
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

    if (is_bool(object)) return object.boolean;

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

    if (a > b) {
        diff = fabs(a - b);
        sum = fabs(a + b);
    } else {
        diff = fabs(b - a);
        sum = fabs(b + a);
    }

    if (diff < (2 * EPSILON * sum)) return true;
    return false;
}

bool
is_almost_equal(Object a, Object b)
{
    if (a.type == INVALID_TOKEN_INT && b.type == INVALID_TOKEN_INT) return true;
    if (a.type == INVALID_TOKEN_INT || b.type == INVALID_TOKEN_INT) return false;
    if (a.type == NIL && b.type == NIL) return true;
    if (a.type == NIL || b.type == NIL) return false;

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

    if (a.type == STRING && b.type == STRING) {
        int z = strcmp(a.string, b.string);
        if (z == 0) return true;
        return false;
    }
    return is_floating_almost_equal(a.number, b.number);
}

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
            return (Object){ .number = -right.number,
                             .type = expr->unary->Operator.type };
        case BANG: {
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
            return (Object){ .number = left.number - right.number, .type = NUMBER };

        case PLUS:
            if ((left.type == NUMBER) && (right.type == NUMBER)) {
                return (Object){ .number = left.number + right.number };
            }
            if ((left.type == STRING) && (right.type == STRING)) {
                char* bigstr = strdup(left.string);
                bigstr = realloc(bigstr, left.string_len + right.string_len + 2);
                if (strncat(bigstr, right.string, right.string_len) != 0) {
                    error(expr->binary->left->literal->value.line,
                          "Unable to concat strings");
                }
                return (Object){ .string = bigstr, .type = STRING };
            }
            break;

        case SLASH:
            return (Object){ .number = left.number / right.number, .type = NUMBER };

        case MOD:
            return (Object){ .number = fmod(left.number, right.number),
                             .type = NUMBER };

        case STAR:
            return (Object){ .number = left.number * right.number, .type = NUMBER };

        case GREATER: {
            bool what = isgreater(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case GREATER_EQUAL: {
            bool what = isgreaterequal(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case LESS: {
            bool what = isless(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case LESS_EQUAL: {
            bool what = islessequal(left.number, right.number);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case BANG_EQUAL: {
            bool what = !is_almost_equal(left, right);
            return (Object){ .boolean = what, .type = boolean_type(what) };
        }
        case EQUAL_EQUAL: {
            bool what = is_almost_equal(left, right);
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