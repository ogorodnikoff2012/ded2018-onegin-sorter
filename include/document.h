#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef char * line_t;

typedef struct {
    char* data;
    int data_size;
    line_t* lines;
    int lines_cnt;
} document_t;

document_t* read_document(const char* filename);
bool check_document(const document_t* document, int* err_pos);
bool print_document(const document_t* document, const char* filename);
void close_document(document_t* document);

int32_t symbol_at(const document_t* document, int pos);
