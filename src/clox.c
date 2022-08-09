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

#include <stddef.h>
#include <stdio.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sysexits.h>

#include "ast_printer.h"
#include "evaluator.h"
#include "environment.h"
#include "parser.h"
#include "program.h"
#include "scanner.h"
#include "token.h"
#include "utility.h"

/* run the interpreter
 * Params:
 * @buffer : a null terminated buffer containing lox source code
 * @buf_len : size of the buffer to be interpreted
 */
void
run(const char* buffer, size_t buf_len, Program* program)
{
    *program->scanner = init_scanner(buffer, buf_len);
    scan_tokens(program->scanner);

    *program->parser = init_parser(program->scanner->tokens);
    Statement* stmts = parse(program);

    program->statements = stmts;

    program->toks_list =
      realloc(program->toks_list, sizeof(Token*) * (program->toks_list_cnt + 1));
    program->tok_cnt =
      realloc(program->tok_cnt, sizeof(size_t) * (program->toks_list_cnt + 1));

    program->toks_list[program->toks_list_cnt] = program->scanner->tokens;
    program->tok_cnt[program->toks_list_cnt++] = program->scanner->tokens_count;

    if (program->parser->had_error) goto expr_end;

    interpret(program);

    for (size_t i = 0; i < program->parser->statements[0].count; i++) {
        if (program->parser->statements[i].type != BLOCK_STMT)
            deallocate_expr(program->parser->statements[i].exStmt.expression);
    }
    free(program->statements);
expr_end:
    free((void*)buffer);
}

/* run the interpreter with a file
 * Params:
 * @filename : the name of file to be interpreted
 */
void
runfile(const char* filename, Program* program)
{
    size_t filesize = 0;
    const char* filebuffer = readfile(filename, &filesize);
    run(filebuffer, filesize, program);

    if (program->parser->had_error) exit(EX_DATAERR);
    if (program->had_runtime_error) exit(EX_SOFTWARE);
}

/* run the interpreter as a RPEL*/
void
run_prompt(Program* program)
{
    for (;;) {
        char* line = readline("> ");
        if (line == NULL) return;

        add_history(line);
        run(line, strlen(line), program);
        program->parser->had_error = false;
    }
}

int
main(int argc, char** argv)
{
    Environment** env = calloc(1, sizeof(Environment*));
    Env_manager env_mgr = { .envs = env, .env_idx = 0 };
    Scanner* scanner = calloc(1, sizeof(Scanner));
    Parser* parser = calloc(1, sizeof(Parser));

    Program program = { .env_mgr = &env_mgr, .parser = parser, .scanner = scanner };

    sh_new_arena(program.env_mgr->envs[0]);
    env_mgr.total_envs++;

    if (argc > 2) {
        fprintf(stderr, "Usage: clox [script]\n");
        exit(EX_USAGE);
    } else if (argc == 2) {
        runfile(argv[1], &program);
    }

    run_prompt(&program);

    shfree(env_mgr.envs[0]);

    for_range(i, program.toks_list_cnt)
      deallocate_tokens(program.toks_list[i], program.tok_cnt[i]);

    free(program.toks_list);
    free(program.tok_cnt);
    free(program.env_mgr->envs);
    free(program.scanner);
    free(program.parser);
}