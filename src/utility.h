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

#ifndef CLOX_BASIC_UTILTIY_H
#define CLOX_BASIC_UTILTIY_H

#include <stdbool.h>
#include <stddef.h>

/* define some popular escape sequences */
/* visit https://github.com/dylanaraps/pure-bash-bible#text-colors for more info
 */
#define RESET "\033[m"

#define BOLD "\033[1m"
#define FAINT "\033[2m"
#define ITALIC "\033[3m"
#define ULINE "\033[4m"
#define BLINK "\033[5m"
#define HIGHLIGHT "\033[7m"

#define GREEN "\033[38;2;0;255;0m"
#define GREEN_2 "\033[32m"
#define RED "\033[38;2;255;0;0m"
#define RED_2 "\033[31m"
#define BLUE "\033[34m"

#define UNUSED(X) (void)X

/* reads a file into a heap-allocated buffer and
 * returns the pointer after reading the file */
char*
readfile(const char* filename, size_t* filesize);

/* returns a heap allocated null terminated char array containing contents
 * from str[start] to str[end]*/
char*
get_substr(const char* str, size_t start, size_t end);

/* report an error */
void
error(int line, const char* message);

/* non-lexicographic string compare
 * compares upto 'count' characters or until a zero byte is hit */
bool
strncmp_nl(const char* s1, const char* s2, size_t count);

/* gets the enum TOKEN_TYPE value of the keyword from the str passed to it
 * if str matches to a keyword */
enum TOKEN_TYPE
get_keyword(const char* str);
#endif