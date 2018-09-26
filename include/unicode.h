#pragma once

/** @file */

#include "unicode_tables.h"

/** Read next utf8-encoded symbol from char buffer and move the pointer
 * \param [in, out] pos Is changed if there is a valid encoded symbol
 * \return code of symbol or -1 if it is invalid */
int32_t next_symbol(const char** pos);

/** Compare two utf8-encoded strings ignoring all symbols except letters and case-insensitively
 *  and return negative, zero or positive value if first string is less, equal or greater
 *  than second string in terms of lexicographical order.
 * */
int unicode_lex_cmp(const void* str_a, const void* str_b);

/** Compare two utf8-encoded strings ignoring all symbols except letters and case-insensitively
 *  and return negative, zero or positive value if first string is less, equal or greater
 *  than second string in terms of reversed lexicographical order.
 * */
int unicode_rev_lex_cmp(const void* str_a, const void* str_b);
