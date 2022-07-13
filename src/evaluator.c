#include <stdbool.h>

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

Object
evaluate_literal(Expr* expr)
{
    return get_object_from_literal(expr);
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
evaluate(Expr* expr)
{
    switch (expr->type) {
        case LITERAL:
            return evaluate_literal(expr);
        case UNARY:
            return evaluate_unary(expr);
        default:
            __builtin_unreachable();
    }
}