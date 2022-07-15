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

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"
#include "token.h"
#include "utility.h"

/* initialise the scanner
 * Params:
 * @source : the source code buffer for lox
 * @source_length : length of the source string
 * Ret:
 * @Scanner : returns a newly initialised scanner structure
 */
Scanner
init_scanner(const char* source, const size_t source_length)
{
    Scanner scanner = { .source = source,
                        .token_max = TOKEN_CNT,
                        .tokens_count = 0,
                        .source_length = source_length,
                        .start = 0,
                        .current = 0,
                        .line = 1 };

    scanner.tokens = allocate_tokens(TOKEN_CNT); // allocate TOKEN_CNT space

    return scanner;
}

/* returns the absolute value of a double
 * Params:
 * @d : value of double to be absolute'd
 * Ret:
 * @double : absolute value of d
 */
static double
abs_d(double d)
{
    if (d < 0.0) return -d;
    return d;
}

static size_t
find_col(Scanner* scanner)
{
    return scanner->start - scanner->line_start_column;
}

/* add a new token to scanner's token array
 * @scanner : the scanner structure
 * @type : the type of token to be added
 * @start : the location where the token in lox's source starts
 * @end : the location where the token in lox's source ends
 */
static void
add_token(Scanner* scanner, enum TOKEN_TYPE type, size_t start, size_t end)
{
    if (type == ENDOF) {
        /* token count is not incremented here because it is the last token */
        scanner->tokens[scanner->tokens_count] =
          init_tok(type, "", 0, 0, scanner->line, find_col(scanner) + 1);

        return;
    }

    /* get the token as a string from the lox source */
    char* text = get_substr(scanner->source, start, end, NULL);

    /* perform conversion if we have a NUMBER token */
    if (type == NUMBER) {
        scanner->tokens[scanner->tokens_count++] =
          init_tok(type,
                   text,
                   end - start,
                   abs_d(strtod(text, NULL)),
                   scanner->line,
                   find_col(scanner));

        return;
    }

    scanner->tokens[scanner->tokens_count++] =
      init_tok(type, text, end - start, 0, scanner->line, find_col(scanner));
}

/* returns if the scanner has reached the end of lox source
 * Params :
 * @scanner : the scanner structure
 * Ret:
 * @bool : true if current >= source_length
 */
static bool
scanner_is_at_end(const Scanner* scanner)
{
    return scanner->current >= scanner->source_length;
}

/* advance_scanners the scanner
 * Params:
 * @scanner : the scanner structure
 * Ret :
 * @char :  the next character in the lox source
 */
static char
advance_scanner(Scanner* scanner)
{
    return scanner->source[scanner->current++];
}

/* matches an expected character with the
 * actual next character in lox source
 * Params:
 * @scanner : the scanner structure
 * @expected : the expected character
 * Ret :
 * @bool : if matched then return true, otherwise false*/
static bool
match(Scanner* scanner, const char expected)
{
    if (scanner_is_at_end(scanner)) return false;
    if (scanner->source[scanner->current] != expected) return false;

    scanner->current++;
    return true;
}

/* character_peeks at the next character in lox source without advancing()
 * Params :
 * @scanner : the scanner structure
 * Ret :
 * @char : the character_peeked character
 */
static char
character_peek(const Scanner* scanner)
{
    if (scanner_is_at_end(scanner)) return '\0';

    return scanner->source[scanner->current];
}

/* same as character_peek() but peeks at the 2nd next character
 * Params :
 * @scanner : the scanner structure
 * Ret :
 * @char : the character_peeked character
 */
static char
character_peek_next(const Scanner* scanner)
{
    if (scanner->current + 1 >= scanner->source_length) return '\0';

    return scanner->source[scanner->current + 1];
}

/* handles strings in lox source code
 * Params :
 * @scanner : the scanner structure
 */
static void
string(Scanner* scanner)
{
    while (character_peek(scanner) != '"' && (!scanner_is_at_end(scanner))) {
        if (character_peek(scanner) == '\n') scanner->line++;
        advance_scanner(scanner);
    }

    if (scanner_is_at_end(scanner)) {
        error(scanner->line, "Unterminated String");
        return;
    }

    /* advance_scanner() to the closing '"' */
    advance_scanner(scanner);

    /* add the token with removed surrounding quotes from the string */
    add_token(scanner, STRING, scanner->start + 1, scanner->current - 1);
}

/* handles numbers in lox source code
 * Params :
 * @scanner : the scanner structure
 */
static void
number(Scanner* scanner)
{
    while (isdigit(character_peek(scanner)))
        advance_scanner(scanner);

    if (character_peek(scanner) == '.' && isdigit(character_peek_next(scanner))) {
        advance_scanner(scanner);

        while (isdigit(character_peek(scanner)))
            advance_scanner(scanner);
    }

    add_token(scanner, NUMBER, scanner->start, scanner->current);
}

/* handles identifiers in lox source code
 * Params :
 * @scanner : the scanner structure
 */
static void
identifier(Scanner* scanner)
{
    while (isalnum(character_peek(scanner)))
        advance_scanner(scanner);

    const char* text =
      get_substr(scanner->source, scanner->start, scanner->current, NULL);

    enum TOKEN_TYPE type = get_keyword(text);

    free((void*)text);

    add_token(scanner, type, scanner->start, scanner->current);
}

/* scans a unit i.e a single token from the lox source code
 * Params :
 * @scanner : the scanner structure
 */
static void
scan_unit_token(Scanner* scanner)
{
    char c = advance_scanner(scanner);
    switch (c) {
        case '(':
            add_token(scanner, LEFT_PAREN, scanner->start, scanner->current);
            break;
        case ')':
            add_token(scanner, RIGHT_PAREN, scanner->start, scanner->current);
            break;
        case '{':
            add_token(scanner, LEFT_BRACE, scanner->start, scanner->current);
            break;
        case '}':
            add_token(scanner, RIGHT_BRACE, scanner->start, scanner->current);
            break;
        case ',':
            add_token(scanner, COMMA, scanner->start, scanner->current);
            break;
        case '.':
            add_token(scanner, DOT, scanner->start, scanner->current);
            break;
        case '-':
            add_token(scanner, MINUS, scanner->start, scanner->current);
            break;
        case '+':
            add_token(scanner, PLUS, scanner->start, scanner->current);
            break;
        case ';':
            add_token(scanner, SEMICOLON, scanner->start, scanner->current);
            break;
        case '*':
            add_token(scanner, STAR, scanner->start, scanner->current);
            break;
        case '%':
            add_token(scanner, MOD, scanner->start, scanner->current);
            break;
        case '!': {
            enum TOKEN_TYPE type = match(scanner, '=') ? BANG_EQUAL : BANG;
            add_token(scanner, type, scanner->start, scanner->current);
        } break;
        case '=': {
            enum TOKEN_TYPE type = match(scanner, '=') ? EQUAL_EQUAL : EQUAL;
            add_token(scanner, type, scanner->start, scanner->current);
        } break;
        case '<': {
            enum TOKEN_TYPE type = match(scanner, '=') ? LESS_EQUAL : LESS;
            add_token(scanner, type, scanner->start, scanner->current);
        } break;
        case '>': {
            enum TOKEN_TYPE type = match(scanner, '=') ? GREATER_EQUAL : GREATER;
            add_token(scanner, type, scanner->start, scanner->current);
        } break;
        case '/':
            if (match(scanner, '/')) {
                // A comment in lox goes until the end of line.
                while (character_peek(scanner) != '\n' &&
                       (!scanner_is_at_end(scanner)))
                    advance_scanner(scanner);
            } else if (match(scanner, '*')) {
                /* this is a block comment in lox */
                /* and look, it can go
                   across lines */
                while ((character_peek(scanner) != '*' ||
                        character_peek_next(scanner) != '/') &&
                       (!scanner_is_at_end(scanner))) {

                    /* handle the newlines inside of the block comments */
                    if (character_peek(scanner) == '\n') {
                        scanner->line++;
                        scanner->line_start_column = scanner->current;
                    }
                    advance_scanner(scanner);
                }
                advance_scanner(scanner);
                advance_scanner(scanner);
            } else
                add_token(scanner, SLASH, scanner->start, scanner->current);
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            scanner->line++;
            scanner->line_start_column = scanner->current;
            break;
        case '"':
            string(scanner);
            break;
        default:
            if (isdigit(c)) number(scanner);
            else if (isalpha(c))
                identifier(scanner);
            else
                error(scanner->line, "Unexpected character.");
            break;
    }
}

/* scans all tokens in the lox source code
 * Params :
 * @scanner : the scanner structure
 * Ret (Optional to be saved) :
 * @Token* : A pointer to all the tokens read
 */
Token*
scan_tokens(Scanner* scanner)
{
    while (!scanner_is_at_end(scanner)) {
        if (scanner->tokens_count == scanner->token_max)
            extend_tokens_by(scanner->tokens, scanner->tokens_count, TOKEN_CNT);
        scanner->start = scanner->current;
        scan_unit_token(scanner);
    }

    /* make sure we have space for one more token */
    if (scanner->tokens_count == scanner->token_max)
        extend_tokens_by(scanner->tokens, scanner->tokens_count, 2);

    add_token(scanner, ENDOF, 0, 0);

    return scanner->tokens;
}