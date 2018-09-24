#pragma once

#include "unicode_tables.h"

int32_t next_symbol(const char** pos);
int unicode_lex_cmp(const void* str_a, const void* str_b);
int unicode_rev_lex_cmp(const void* str_a, const void* str_b);
