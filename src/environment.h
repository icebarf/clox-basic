#ifndef CLOX_BASIC_ENVIRONMENT_H
#define CLOX_BASIC_ENVIRONMENT_H

#include <stbds.h>
#include <stddef.h>

#include "program.h"
#include "parser.h"

typedef struct Env_t {
    char* key;
    Object value;
    Environment** enclosing;
} Environment;

Environment*
create_env(void);

Environment
init_env(void);

Environment
init_env_with(Env_manager* envmgr, size_t par_idx);

void
define(Env_manager* env_mgr, char* string, Object value, size_t idx);

void
define_with_struct(Env_manager* env_mgr, Environment, size_t idx);

Object
get_value(Env_manager* env_mgr, Token name, size_t idx);

Object
assign(Env_manager* env_mgr, Token name, Object value, size_t idx);

#endif