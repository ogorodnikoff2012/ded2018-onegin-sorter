#pragma once

#include <stdbool.h>

typedef char * line_t;

typedef struct {
    char* data;
    int data_size;
    line_t* lines;
    int lines_cnt;
} document_t;

document_t* read_document(const char* filename);
bool print_document(const document_t* document, const char* filename);
void close_document(document_t* document);
