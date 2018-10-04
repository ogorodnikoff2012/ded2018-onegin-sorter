#!/usr/bin/python3
# See description at ftp://unicode.org/Public/3.0-Update/UnicodeData-3.0.0.html

FIELD_CODE_VALUE = 0
FIELD_GENERAL_CATEGORY = 2
FIELD_UNICODE_UPPERCASE_MAPPING = 12
FIELD_UNICODE_LOWERCASE_MAPPING = 13

UNICODE_BLOCK_BASIC_LATIN = (0x00, 0x7F)
UNICODE_BLOCK_LATIN_1_SUPPLEMENT = (0x80, 0xFF)
UNICODE_BLOCK_CYRILLIC = (0x400, 0x4FF)

ALLOWED_BLOCKS_MASK = 0x4FF
UTF8_ALLOWED_SYMBOLS_MASK = 0x10FFFF
MAX_ALLOWED_SYMBOL = ALLOWED_BLOCKS_MASK

CATEGORY = {'L': (0, 'LETTER'), 'M': (1, 'MARK'), 'N': (2, 'NUMBER'), 'P': (3, 'PUNCTUATION'), 'S': (4, 'SYMBOL'), 'Z': (5, 'SEPARATOR'), 'C': (6, 'OTHER')}

def in_block(code, block):
    return code >= block[0] and code <= block[1]

database = dict()

while True:
    try:
        s = input()
        row = s.split(';')
        code_value, uppercase_mapping, lowercase_mapping = map(lambda x: -1 if x == '' else int(x, 16),
                [row[FIELD_CODE_VALUE], row[FIELD_UNICODE_UPPERCASE_MAPPING], row[FIELD_UNICODE_LOWERCASE_MAPPING]])
        general_category = row[FIELD_GENERAL_CATEGORY]
        if in_block(code_value, UNICODE_BLOCK_BASIC_LATIN) or in_block(code_value, UNICODE_BLOCK_CYRILLIC) or in_block(code_value, UNICODE_BLOCK_LATIN_1_SUPPLEMENT):
            database[code_value] = (general_category, uppercase_mapping, lowercase_mapping)
    except EOFError:
        break

with open('unicode_tables.h', 'w') as f:
    print("""#ifndef UNICODE_PARTIAL_PARSER_H
#define UNICODE_PARTIAL_PARSER_H
/* This is an autogenerated file. Do not modify! */
/* This is a Unicode parser that can work only with Basic Latin, Latin-1 Supplement and Cyrillic Unicode blocks */

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdbool.h>
#include <stdint.h>
#endif /* __cplusplus */

typedef uint32_t unicode_t;
#define ALLOWED_BLOCKS_MASK""", hex(ALLOWED_BLOCKS_MASK), """
#define UTF8_ALLOWED_SYMBOLS_MASK""", hex(UTF8_ALLOWED_SYMBOLS_MASK), file=f)
    print("""
typedef enum {
""", file=f)
    for cat in CATEGORY.values():
        print("    CAT_" + cat[1].upper() + " =", cat[0], ",", file=f)
    print("""
} general_category_t;

static inline bool is_allowed(const unicode_t symbol) {
    return (~ALLOWED_BLOCKS_MASK & symbol) == 0;
}

static inline bool is_valid(const unicode_t symbol) {
    return (~UTF8_ALLOWED_SYMBOLS_MASK & symbol) == 0;
}

void check_for_validity(const char* str);

extern const unicode_t UNICODE_UPPER[];
extern const unicode_t UNICODE_LOWER[];
extern const general_category_t CATEGORY[];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UNICODE_PARTIAL_PARSER_H */""", file=f)

with open('unicode_tables.c', 'w') as f:
    print("""#include "unicode_tables.h" /* If needed, replace it */
/* This file is autogenerated */

const unicode_t UNICODE_UPPER[] = {""", file=f)
    for n in range(MAX_ALLOWED_SYMBOL + 1):
        upper = database.get(n, ('C', -1, -1))[1]
        print(hex(upper if upper >= 0 else n), ',', file=f)
    print("""};

const unicode_t UNICODE_LOWER[] = {""", file=f)
    for n in range(MAX_ALLOWED_SYMBOL + 1):
        lower = database.get(n, ('C', -1, -1))[2]
        print(hex(lower if lower >= 0 else n), ',', file=f)
    print("""};

const general_category_t CATEGORY[] = {""", file=f)
    for n in range(MAX_ALLOWED_SYMBOL + 1):
        cat = database.get(n, ('C', -1, -1))[0][0]
        print('CAT_' + CATEGORY[cat][1], ',', file=f)
    print("""};""", file=f)
