#include "scanner.h"

Scanner init_scanner(const char* source, const size_t source_length)
{
    Scanner scanner = {.source = source,
                       .source_length = source_length,
                       .start = 0,
                       .current = 0,
                       .line = 1};
    return scanner;
}

_Bool is_at_end(const Scanner scanner)
{
    return scanner.current >= scanner.source_length;
}