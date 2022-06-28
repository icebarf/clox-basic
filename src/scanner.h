#ifndef CLOX_BASIC_SCANNER_H
#define CLOX_BASIC_SCANNER_H

#include "token.h"
#include <stddef.h>

typedef struct Scanner {
    const char* source;
    Token* tokens;
    size_t source_length;
    size_t start;
    size_t current;
    size_t line;
} Scanner;

/* initialise the scanner */
Scanner init_scanner(const char* source, const size_t source_length);

#endif