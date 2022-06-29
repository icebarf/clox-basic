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

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>

#include "token.h"

extern bool had_error;

char* readfile(const char* filename)
{
    /* open a file stream to filename*/
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file with fopen(): %s\n",
                strerror(errno));
        exit(EX_OSERR);
    }

    /* goto the end of the file stream */
    if (fseek(file, 0L, SEEK_END) != 0) {
        fprintf(stderr, "Error: fseek() could not seek to end of file\n");
        exit(EX_UNAVAILABLE);
    }

    /* obtain the how big the file is */
    long int sz = ftell(file);
    if (sz == -1L) {
        fprintf(stderr,
                "Error: ftell() could not obtain position in file: %s\n",
                strerror(errno));
        exit(EX_UNAVAILABLE);
    }

    /* go back to the beginning */
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fprintf(stderr,
                "Error: fseek() could not seek back to beginning of file\n");
        exit(EX_UNAVAILABLE);
    }

    /* allocate a buffer to hold the user specified file in memory */
    char* filedata = malloc(sz + 1);
    if (filedata == NULL) {
        fprintf(stderr, "Error: malloc(%ld) returned NULL\n", sz + 1);
        exit(EX_OSERR);
    }

    /* read the file into the buffer */
    if (fread(filedata, sizeof(char), sz, file) != (unsigned long)sz) {
        if (feof(file)) {
            fprintf(stderr,
                    "Error: reading data with fread(): Early end-of-file "
                    "indicator in file "
                    "stream\n");
            exit(EX_IOERR);
        }
        if (ferror(file)) {
            fprintf(stderr,
                    "Error: reading data with fread(): error indicator in file "
                    "stream\n");
            exit(EX_IOERR);
        }
        fprintf(stderr, "Error: reading data with fread(): Unknown error\n");
        exit(EX_IOERR);
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "Error: fclose() could not close file stream: %s\n",
                strerror(errno));
        exit(EX_OSERR);
    }

    return filedata;
}

const char* get_substr(const char* str, size_t start, size_t end)
{
    char* substr = malloc(end + 1);
    size_t cnt = 0;
    while (cnt <= end) {
        substr[cnt] = str[start + cnt];
        cnt++;
    }
    substr[cnt] = '\0';

    return substr;
}

void report(int line, const char* where, const char* message)
{
    fprintf(stderr, "[line %d ] Error %s : %s\n", line, where, message);
}

void error(int line, const char* message)
{
    report(line, "", message);
    had_error = true;
}