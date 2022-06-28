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

#include <stdio.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <sysexits.h>

#include "scanner.h"
#include "token.h"
#include "utility.h"

/* temporary run function until all required functions have been completed */
void run(const char* buffer)
{
    puts(buffer);
    free((void*)buffer);
}

void runfile(const char* filename)
{
    const char* filebuffer = readfile(filename);
    run(filebuffer);
}

void run_prompt(void)
{
    for (;;) {
        const char* line = readline("> ");
        if (line == NULL)
            break;

        add_history(line);
        run(line);
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