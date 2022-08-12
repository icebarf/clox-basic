#pragma once
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

#ifndef CLOX_BASIC_PARSER_H
#define CLOX_BASIC_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "token.h"

typedef struct Expr_t Expr;
typedef struct Program_t Program;
typedef struct Env_t Environment;
typedef struct {
    Environment** envs;
    size_t env_idx;
    size_t total_envs;
} Env_manager;

typedef struct Object_t {
    union {
        double number;
        bool boolean;
    };
    char* string;
    size_t string_len;
    enum TOKEN_TYPE type;
} Object;

/* atomic structures that make up the expression structure */
struct Binary_e {
    Token Operator;
    Expr* left;
    Expr* right;
    // visitor pattern. Make the expression visit any function.
    void (*accept)(Env_manager* env_mgr, struct Binary_e*);
    bool nests;
};

struct Grouping_e {
    Expr* expression;
    void (*accept)(Env_manager* env_mgr, struct Grouping_e*);
};

struct Unary_e {
    Token Operator;
    Expr* right;
    void (*accept)(Env_manager* env_mgr, struct Unary_e*);
};

struct Literal_e {
    Token value;
    void (*accept)(Env_manager* env_mgr, struct Literal_e*);
};

struct Variable_e {
    Token name;
    Expr* value;
    void (*accept)(Env_manager* env_mgr, struct Variable_e*);
};

/* the molecule - expression */
struct Expr_t {
    union {
        struct Binary_e* binary;
        struct Grouping_e* group;
        struct Unary_e* unary;
        struct Literal_e* literal;
        struct Variable_e* variable;
    };
    void (*accept)(Expr*);
    Object (*evaluate)(Env_manager* env_mgr, Expr*, bool*);
    enum EXPR_TYPES {
        LITERAL,
        UNARY,
        BINARY,
        GROUPING,
        VARIABLE,
        INVALID_EXPR_INT
    } type;
};

typedef struct {
    Token tok;
    Expr* expression;
} Expr_statement;

typedef Expr_statement Print_statement;
typedef Expr_statement Var_decl;
typedef struct Statement_t Statement;

typedef struct {
    Statement* statements;
} Block;

typedef struct {
    Expr* condition;
    Statement* branches;
    enum { THEN_BRNCH, ELSE_BRNCH } ran;
} If_stmt;

struct Statement_t {
    union {
        Expr_statement exStmt;
        Print_statement prtStmt;
        Var_decl vardecl;
        If_stmt ifStmt;
        Block block;
    };
    void (*accept)(Env_manager* env_mgr, Statement, bool*);
    size_t count;
    size_t env_idx;
    enum STMT_TYPE {
        EXPR_STMT,
        PRINT_STMT,
        VAR_DECL_STMT,
        IF_STMT,
        BLOCK_STMT,
        BAD_STMT
    } type;
};

typedef struct {
    Token* tokens;
    Statement* statements;
    size_t current_token_idx;
    size_t current_statement_idx;
    bool had_error;
} Parser;

/******* Functions ********/

/* initialise the parser */
Parser
init_parser(Token* tokens);

/* expression allocator */
void*
allocate_expr(void);

/* expression de-allocator */
void
deallocate_expr(Expr* expr);

Statement*
parse(Program* program);

#endif