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