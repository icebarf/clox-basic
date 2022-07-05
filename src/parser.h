#ifndef CLOX_BASIC_PARSER_H
#define CLOX_BASIC_PARSER_H

#include <stdbool.h>

#include "token.h"

typedef struct Expr_t Expr;

struct Expr_t {
    enum EXPR_TYPES { LITERAL, UNARY, BINARY, GROUPING } type;
    union Holder {
        struct Binary_e {
            Expr* left;
            Token* Operator;
            Expr* right;
        } Binary_e;

        struct Grouping_e {
            Expr* expression;
        } Grouping_e;

        struct Unary_e {
            Token* Operator;
            Expr* right;
        } Unary_e;

        struct Literal_e {
            enum TOKEN_TYPE type;
            union {
                double number;
                char* string;
                bool boolean;
            } value;
        } Literal_e;
    } hld;
};

#endif