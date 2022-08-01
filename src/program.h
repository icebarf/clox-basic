#ifndef CLOX_BASIC_PROGRAM_STRUCTURES_H
#define CLOX_BASIC_PROGRAM_STRUCTURES_H

#include "parser.h"
#include "scanner.h"
#include <stddef.h>

typedef struct Program_t {
    Scanner* scanner;
    Parser* parser;
    Statement* statements;
    Env_manager* env_mgr;
    Token** toks_list;
    size_t* tok_cnt;
    size_t toks_list_cnt;
    bool had_runtime_error;
} Program;

enum { GLOBAL_ENV };

#endif