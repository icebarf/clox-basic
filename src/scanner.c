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