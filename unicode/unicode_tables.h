#ifndef UNICODE_PARTIAL_PARSER_H
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
#define ALLOWED_BLOCKS_MASK 0x4ff 
#define UTF8_ALLOWED_SYMBOLS_MASK 0x10ffff

typedef enum {

    CAT_LETTER = 0 ,
    CAT_MARK = 1 ,
    CAT_NUMBER = 2 ,
    CAT_PUNCTUATION = 3 ,
    CAT_SYMBOL = 4 ,
    CAT_SEPARATOR = 5 ,
    CAT_OTHER = 6 ,

} general_category_t;

static inline bool is_allowed(const unicode_t symbol) {
    return (~ALLOWED_BLOCKS_MASK & symbol) == 0;
}

static inline bool is_valid(const unicode_t symbol) {
    return (~UTF8_ALLOWED_SYMBOLS_MASK & symbol) == 0;
}

void check_for_validity(const char* str);

extern const unicode_t UPPER[];
extern const unicode_t LOWER[];
extern const general_category_t CATEGORY[];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UNICODE_PARTIAL_PARSER_H */
