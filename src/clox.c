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
#include "parser.h"
#include "scanner.h"
#include "token.h"
#include "utility.h"

/* do not execute code that has an error */
bool had_error = false; /* bad - need to figure out a better way */

/* run the interpreter
 * Params:
 * @buffer : a null terminated buffer containing lox source code
 * @buf_len : size of the buffer to be interpreted
 */
void
run(const char* buffer, size_t buf_len)
{
    Scanner scanner = init_scanner(buffer, buf_len);

    scan_tokens(&scanner);

    for (size_t i = 0; i <= scanner.tokens_count; i++) {
        const char* str = token_to_str(&scanner.tokens[i]);
        fprintf(stdout, "%s\n", str);
        free((void*)str);
    }

    // Unary Expression
    Token Operator = init_tok(BANG, "!", 0, 0);

    bool bull = false;
    struct Literal_e literal =
      init_literal_expression(FALSE, &bull, &literal_to_str);

    Expr right = init_expression(LITERAL, &literal, &print_expr);

    struct Unary_e unary = init_unary_expr(&Operator, &right, &unary_to_str);

    Expr unary_expr = init_expression(UNARY, &unary, &print_expr);

    // A little more complex expression

    double number = 10;
    struct Literal_e left_lit =
      init_literal_expression(NUMBER, &number, &literal_to_str);

    struct Literal_e right_lit =
      init_literal_expression(NUMBER, &number, &literal_to_str);

    Token Op = init_tok(EQUAL_EQUAL, "==", 0, 0);

    Expr left_exp = init_expression(LITERAL, &left_lit, &print_expr);

    Expr right_exp = init_expression(LITERAL, &right_lit, &print_expr);

    struct Binary_e bin =
      init_binary_expr(&left_exp, &Op, &right_exp, &binary_to_str);

    Expr binary_expr = init_expression(BINARY, &bin, &print_expr);

    // Group
    struct Grouping_e grp = init_group_expr(&binary_expr, &grouping_to_str);
    struct Grouping_e grp1 = init_group_expr(&unary_expr, &grouping_to_str);

    Expr grp_expr = init_expression(GROUPING, &grp, &print_expr);

    Expr grp1_expr = init_expression(GROUPING, &grp1, &print_expr);

    // binary expression of groups
    Token op = init_tok(BANG_EQUAL, "!=", 0, 0);
    struct Binary_e binary_structure =
      init_binary_expr(&grp_expr, &op, &grp1_expr, &binary_to_str);
    Expr binary_expression = init_expression(BINARY, &binary_structure, &print_expr);

    fprintf(stdout, "\n( ");
    binary_expression.accept(&binary_expression);
    fprintf(stdout, " )\n");

    free((void*)buffer);
    deallocate_tokens(scanner.tokens, scanner.tokens_count);
}

/* run the interpreter with a file
 * Params:
 * @filename : the name of file to be interpreted
 */
void
runfile(const char* filename)
{
    size_t filesize = 0;
    const char* filebuffer = readfile(filename, &filesize);
    run(filebuffer, filesize);

    if (had_error)
        exit(EX_DATAERR);
}

/* run the interpreter as a RPEL*/
void
run_prompt(void)
{
    for (;;) {
        char* line = readline("> ");
        if (line == NULL)
            return;

        add_history(line);
        run(line, strlen(line));
        had_error = false;
    }
}

int
main(int argc, char** argv)
{
    if (argc > 2) {
        fprintf(stderr, "Usage: clox [script]\n");
        exit(EX_USAGE);
    } else if (argc == 2) {
        runfile(argv[1]);
    }

    run_prompt();
}