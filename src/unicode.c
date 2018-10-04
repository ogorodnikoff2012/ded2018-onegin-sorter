#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "unicode.h"
#include "macros.h"

typedef int32_t (* utf8_decoder_t)(const char*); /* Pointer to a function: (const char*) -> int32_t */

/* Parsers for different sizes of utf8 symbol: it's easier to use byte magic rather than loops and if branches */
static int32_t next_symbol_i1(const char*);
static int32_t next_symbol_i2(const char*);
static int32_t next_symbol_i3(const char*);
static int32_t next_symbol_i4(const char*);
static int32_t next_symbol_invalid(const char*);

const static int BYTES[] = {
    /* 0xxxxxx */
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    /* 10xxxxxx - INVALID FIRST BYTE */
    0, 0, 0, 0, 0, 0, 0, 0,
    /* 110xxxxx */
    2, 2, 2, 2,
    /* 1110xxxx */
    3, 3,
    /* 11110xxx */
    4,
    /* 11111xxx - INVALID BYTE */
    0
};

const static utf8_decoder_t NEXT_SYMBOL_INTERNAL[] = {
    next_symbol_invalid,
    next_symbol_i1,
    next_symbol_i2,
    next_symbol_i3,
    next_symbol_i4,
};

/* 0xxxxxxx */
static int32_t next_symbol_i1(const char* pos) {
    return pos[0] & 0x7F;
}

/* 110xxxxx 10xxxxxx */
static int32_t next_symbol_i2(const char* pos) {
    return ((int32_t)(pos[0] & 0x1F) << 6) | (pos[1] & 0x3F);
}

/* 1110xxxx 10xxxxxx 10xxxxxx */
static int32_t next_symbol_i3(const char* pos) {
    return ((int32_t)(pos[0] & 0x0F) << 12) | ((int32_t)(pos[1] & 0x3F) << 6) | (pos[2] & 0x3F);
}

/* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
static int32_t next_symbol_i4(const char* pos) {
    return ((int32_t)(pos[0] & 0x07) << 18) | ((int32_t)(pos[1] & 0x3F) << 12) | ((int32_t)(pos[2] & 0x3F) << 6) | (pos[3] & 0x3F);
}

static int32_t next_symbol_invalid(const char* pos) {
    UNUSED(pos);
    return -1;
}

/* ch == 10xxxxxx */
static inline bool intermediate(char ch) {
    return (ch & 0xC0) == 0x80;
}

#define PARSE_AND_MOVE(delta) \
    int bytes = BYTES[(uint8_t)(**pos) >> 3]; \
    int32_t result = NEXT_SYMBOL_INTERNAL[bytes](*pos); \
    *pos += ( delta ); \
    return result;

int32_t next_symbol(const char** pos) {
    PARSE_AND_MOVE(bytes)
}

int32_t prev_symbol(const char** pos) {
    do {
        --(*pos);
    } while (intermediate(**pos));

    PARSE_AND_MOVE(0)
}

#define FIND_LETTER(func_prefix) \
static int32_t func_prefix##_letter(const char** pos) { \
    int32_t symbol; \
    do { \
        symbol = func_prefix##_symbol(pos); \
    } while (symbol > 0 && (!is_allowed(symbol) || (CATEGORY[symbol] != CAT_LETTER))); \
    return symbol; \
}

FIND_LETTER(next)
FIND_LETTER(prev)

/* We don't have templates in C :-( */
#define COMPARATOR(name, iterator, from_end, while_predicate) \
int name (const void* a, const void* b) { \
    const char* str_a = *(const char **)a; \
    const char* str_b = *(const char **)b; \
    const char* iter_a = str_a; \
    const char* iter_b = str_b; \
    if ( from_end ) { \
        iter_a += strlen(str_a); \
        iter_b += strlen(str_b); \
    } \
    int32_t a_symbol, b_symbol; \
    do { \
        a_symbol = UNICODE_UPPER[ iterator (&iter_a)]; \
        b_symbol = UNICODE_UPPER[ iterator (&iter_b)]; \
    } while ( while_predicate ); \
    if (from_end && (a_symbol == b_symbol)) { \
        a_symbol = (iter_a == str_a) ? 0 : UNICODE_UPPER[ iterator (&iter_a)]; \
        b_symbol = (iter_b == str_b) ? 0 : UNICODE_UPPER[ iterator (&iter_b)]; \
    } \
    return a_symbol - b_symbol; \
}

COMPARATOR(unicode_lex_cmp, next_letter, false, (a_symbol == b_symbol && a_symbol != 0))
COMPARATOR(unicode_rev_lex_cmp, prev_letter, true, (a_symbol == b_symbol && iter_a != str_a && iter_b != str_b))
