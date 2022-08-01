#include "parser.h"
#include "token.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#define STB_DS_IMPLEMENTATION
#define STBDS_SIPHASH_2_4

#include "environment.h"

void
define(Env_manager* env_mgr, char* string, Object value, size_t idx)
{

    Environment box = { .value = value,
                        .key = string,
                        .enclosing = &env_mgr->envs[env_mgr->env_idx - 1] };
    define_with_struct(env_mgr, box, idx);
}

void
define_with_struct(Env_manager* env_mgr, Environment box, size_t idx)
{
    shputs((env_mgr->envs[idx]), box);
}

bool
key_exists(Env_manager* env_mgr, char* key, size_t idx)
{
    return shgeti((env_mgr->envs[idx]), key) >= 0;
}

Object
get_value(Env_manager* env_mgr, Token name, size_t idx)
{

    if (key_exists(env_mgr, name.lexeme, idx))
        return shgets((env_mgr->envs[idx]), name.lexeme).value;

    if (idx >= 1) return get_value(env_mgr, name, idx - 1);

    return (Object){ .type = INVALID_TOKEN_INT };
}

Object
assign(Env_manager* env_mgr, Token name, Object value, size_t idx)
{

    if (key_exists(env_mgr, name.lexeme, idx)) {
        define(env_mgr, name.lexeme, value, idx);
        return (Object){ .type = VAR };
    }

    if (idx >= 1) {
        assign(env_mgr, name, value, idx - 1);
        return (Object){ .type = VAR };
    }

    return (Object){ .type = INVALID_TOKEN_INT };
}

Environment*
create_env(void)
{
    return malloc(sizeof(Environment));
}

Environment
init_env(void)
{
    return (Environment){ 0 };
}

Environment
init_env_with(Env_manager* envmgr, size_t par_idx)
{

    return (Environment){ .enclosing = &envmgr->envs[par_idx] };
}