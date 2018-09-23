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

bool check_args(int argc, char* argv[]) {
    UNUSED(argv);
    return argc == 2;
}

void print_usage(FILE* stream, const char* progname) {
    fprintf(stream, "Usage: %s <input.txt>\n", progname);
}

int less_cmp(const void* a, const void* b) {
    return *(char **)a - *(char **)b;
}

int main(int argc, char* argv[]) {
    if (!check_args(argc, argv)) {
        print_usage(stderr, argv[0]);
        return 1;
    }
    document_t *document = read_document(argv[1]);
    if (document == NULL) {
        fprintf(stderr, "Cannot read document!\n");
        return 0;
    }

    int output_filename_length = strlen(argv[1]) + TASK_NAMES_MAX_LENGTH + 1;
    char* output_filename = malloc(output_filename_length);
    int last_dot = strrchr(argv[1], '.') - argv[1];
    strncpy(output_filename, argv[1], last_dot);

    for (int i = 0; i < N_TASKS; ++i) {
        qsort(document->lines, document->lines_cnt, sizeof(line_t), COMPARATORS[i]);
        snprintf(output_filename + last_dot, output_filename_length - last_dot, "_%s%s",
                 TASK_NAMES[i], argv[1] + last_dot);
        print_document(document, output_filename);
    }
    free(output_filename);
    close_document(document);

    return 0;
}
