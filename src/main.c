#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "document.h"
#include "unicode.h"

#define UNUSED(x) if (false) { x = x; }

int less_cmp(const void*, const void*);

typedef int (*comparator_t)(const void*, const void*);
const comparator_t COMPARATORS[] = {unicode_lex_cmp, unicode_rev_lex_cmp, less_cmp};
const char* TASK_NAMES[] = {"sorted", "rythms", "original"};
const int N_TASKS = 3;
const int TASK_NAMES_MAX_LENGTH = 40;

typedef struct {
    bool nocheck;
} args_t;

bool parse_args(int argc, char* argv[], args_t* args) {
    args->nocheck = false;
    if (argc == 2) {
        return true;
    }
    if (argc == 3 && strcmp(argv[1], "-n") == 0) {
        args->nocheck = true;
        return true;
    }
    return false;
}

void print_usage(FILE* stream, const char* progname) {
    fprintf(stream, "Usage: %s [-n] <input.txt>\n", progname);
}

int less_cmp(const void* a, const void* b) {
    return *(char **)a - *(char **)b;
}

int main(int argc, char* argv[]) {
    args_t args;
    if (!parse_args(argc, argv, &args)) {
        print_usage(stderr, argv[0]);
        return 1;
    }

    const char* input_filename = argv[argc - 1];

    document_t *document = read_document(input_filename);
    if (document == NULL) {
        fprintf(stderr, "Cannot read document %s!\n", input_filename);
        return 1;
    }
    int err_pos = -1;
    if (!args.nocheck && !check_document(document, &err_pos)) {
        fprintf(stderr, "Bad symbol at file %s at position %d: 0x%x\n", input_filename, err_pos, symbol_at(document, err_pos));
        return 1;
    }

    int output_filename_length = strlen(input_filename) + TASK_NAMES_MAX_LENGTH + 1;
    char* output_filename = malloc(output_filename_length);
    int last_dot = strrchr(input_filename, '.') - input_filename;
    strncpy(output_filename, input_filename, last_dot);

    for (int i = 0; i < N_TASKS; ++i) {
        qsort(document->lines, document->lines_cnt, sizeof(line_t), COMPARATORS[i]);
        snprintf(output_filename + last_dot, output_filename_length - last_dot, "_%s%s",
                 TASK_NAMES[i], input_filename + last_dot);
        print_document(document, output_filename);
    }
    free(output_filename);
    close_document(document);

    return 0;
}
