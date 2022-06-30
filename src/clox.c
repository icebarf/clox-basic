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

#include "scanner.h"
#include "token.h"
#include "utility.h"

/* do not execute code that has an error */
bool had_error = false; /* bad - need to figure out a better way */

/* temporary run function until all required functions have been completed */
void run(const char* buffer, size_t buf_len)
{
    Scanner scanner = init_scanner(buffer, buf_len);

    scan_tokens(&scanner);

    for (size_t i = 0; i <= scanner.tokens_count; i++) {
        const char* str = token_to_str(&scanner.tokens[i]);
        fprintf(stdout, "%s\n", str);
        free((void*)str);
    }

    free((void*)buffer);
    deallocate_tokens(scanner.tokens, scanner.tokens_count);
}

void runfile(const char* filename)
{
    size_t filesize = 0;
    const char* filebuffer = readfile(filename, &filesize);
    run(filebuffer, filesize);

    if (had_error)
        exit(EX_DATAERR);
}

void run_prompt(void)
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

int main(int argc, char** argv)
{
    if (argc > 2) {
        fprintf(stderr, "Usage: clox [script]\n");
        exit(EX_USAGE);
    } else if (argc == 2) {
        runfile(argv[1]);
    }

    run_prompt();
}