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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>

#include "token.h"
#include "utility.h"

extern bool had_error;
extern const char* TokenTypeString[40];

char* readfile(const char* filename, size_t* filesize)
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

    *filesize = sz;

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
    /* return null if the size to get is zero */
    if (end - start == 0)
        return NULL;

    char* substr = malloc((end - start) + 1);
    size_t cnt = 0;
    while (cnt < (end - start)) {
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

bool strncmp_nl(const char* s1, const char* s2, size_t count)
{
    if (s1 == NULL || s2 == NULL)
        return false;
    if (count == 0)
        return false;

    while (count--) {
        if (*s1 == '\0' || *s2 == '\0')
            return false;
        if (*s1++ != *s2++)
            return false;
    }
    return true;
}

enum TOKEN_TYPE get_keyword(const char* str)
{
    if (strncmp_nl(str, TokenTypeString[AND], strlen(TokenTypeString[AND])))
        return AND;
    if (strncmp_nl(str, TokenTypeString[CLASS], strlen(TokenTypeString[CLASS])))
        return CLASS;
    if (strncmp_nl(str, TokenTypeString[ELSE], strlen(TokenTypeString[ELSE])))
        return ELSE;
    if (strncmp_nl(str, TokenTypeString[FALSE], strlen(TokenTypeString[FALSE])))
        return FALSE;
    if (strncmp_nl(str, TokenTypeString[FOR], strlen(TokenTypeString[FOR])))
        return FOR;
    if (strncmp_nl(str, TokenTypeString[FUN], strlen(TokenTypeString[FUN])))
        return FUN;
    if (strncmp_nl(str, TokenTypeString[IF], strlen(TokenTypeString[IF])))
        return IF;
    if (strncmp_nl(str, TokenTypeString[NIL], strlen(TokenTypeString[NIL])))
        return NIL;
    if (strncmp_nl(str, TokenTypeString[OR], strlen(TokenTypeString[OR])))
        return OR;
    if (strncmp_nl(str, TokenTypeString[PRINT], strlen(TokenTypeString[PRINT])))
        return PRINT;
    if (strncmp_nl(str, TokenTypeString[RET], strlen(TokenTypeString[RET])))
        return RET;
    if (strncmp_nl(str, TokenTypeString[SUPER], strlen(TokenTypeString[SUPER])))
        return SUPER;
    if (strncmp_nl(str, TokenTypeString[THIS], strlen(TokenTypeString[THIS])))
        return THIS;
    if (strncmp_nl(str, TokenTypeString[TRUE], strlen(TokenTypeString[TRUE])))
        return TRUE;
    if (strncmp_nl(str, TokenTypeString[VAR], strlen(TokenTypeString[VAR])))
        return VAR;
    if (strncmp_nl(str, TokenTypeString[WHILE], strlen(TokenTypeString[WHILE])))
        return WHILE;
    return IDENTIFIER;
}