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
evaluate(Expr* expr)
{
    UNUSED(expr);
    return (Object){ .string = NULL, .type = NIL };
}