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

#ifndef CLOX_BASIC_EVALUATOR_H
#define CLOX_BASIC_EVALUATOR_H

#include "environment.h"
#include <stdbool.h>

Object
evaluate(Env_manager* env_mgr, Expr* expr, bool* had_runtime_error);

void
eval_expr_stmt(Env_manager* env_mgr, Statement statement, bool* had_runtime_error);

void
eval_print_stmt(Env_manager* env_mgr, Statement statement, bool* had_runtime_error);

void
eval_var_stmt(Env_manager* env_mgr, Statement statement, bool* had_runtime_error);

void
eval_block(Env_manager* env_mgr, Statement statement, bool* had_runtime_error);

void
interpret(Program* program);

#endif