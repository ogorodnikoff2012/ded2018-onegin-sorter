#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "document.h"
#include "unicode.h"
#define DATA_EMPTY_PREFIX 1
#define DATA_EMPTY_SUFFIX 1

static void separate_into_lines(document_t* doc) {
    char* data_begin = doc->data;
    char* data_end = data_begin + doc->data_size;
    char* last_line_begin = data_begin;
    doc->lines_cnt = 0;
    for (char* it = data_begin; it != data_end; ++it) {
        if (*it == '\n' || *it == '\r') {
            *it = '\0';
        }

        if (*it == '\0') {
            if (it != last_line_begin) {
                ++doc->lines_cnt;
            }
            last_line_begin = it + 1;
        }
    }

    doc->lines = calloc(sizeof(line_t), doc->lines_cnt);
    last_line_begin = data_begin;
    int lines_added = 0;
    for (char* it = data_begin; it != data_end; ++it) {
        if (*it == '\0') {
            if (it != last_line_begin) {
                doc->lines[lines_added++] = last_line_begin;
            }
            last_line_begin = it + 1;
        }
    }
}

static void remove_bom(document_t* doc) {
    /* BOM : EF BB BF */
    if (doc->data_size >= 3 + DATA_EMPTY_PREFIX + DATA_EMPTY_SUFFIX &&
        doc->data[1] == '\xEF' && doc->data[2] == '\xBB' && doc->data[3] == '\xBF') {
        for (int i = 1; i <= 3; ++i) {
            doc->data[i] = 0;
        }
    }
}

document_t* read_document(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return NULL;
    }

    struct stat filestat;
    if (fstat(fd, &filestat) == -1) {
        return NULL;
    }
    int filesize = filestat.st_size;

    document_t* doc = calloc(sizeof(document_t), 1);
    doc->data_size = filesize + DATA_EMPTY_PREFIX + DATA_EMPTY_SUFFIX;
    doc->data = malloc(doc->data_size);
    doc->data[filesize + DATA_EMPTY_PREFIX] = doc->data[0] = '\0';
    if (read(fd, doc->data + DATA_EMPTY_PREFIX, filesize) == -1) {
        close_document(doc);
        return NULL;
    }
    close(fd);

    remove_bom(doc);
    separate_into_lines(doc);

    return doc;
}

bool check_document(const document_t* doc, int* err_pos) {
    const char* file_begin = doc->data + DATA_EMPTY_PREFIX;
    const char* iter = file_begin;
    const char* prev_iter = iter;
    int32_t symbol;
    for (int i = 0; i < doc->lines_cnt; ++i) {
        prev_iter = iter = doc->lines[i];
        while ((symbol = next_symbol(&iter)) > 0 && is_allowed(symbol)) { prev_iter = iter; }
        if (symbol == 0) {
            continue;
        }
        *err_pos = prev_iter - file_begin;
        return false;
    }
    return true;
}

int symbol_at(const document_t* doc, int pos) {
    const char* iter = doc->data + pos + DATA_EMPTY_PREFIX;
    return next_symbol(&iter);
}

bool print_document(const document_t* doc, const char* filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        return false;
    }
    for (int i = 0; i < doc->lines_cnt; ++i) {
        if (write(fd, doc->lines[i], strlen(doc->lines[i])) == -1 ||
            write(fd, "\n", 1) == -1) {
            close(fd);
            return false;
        }
    }
    close(fd);
    return true;
}

void close_document(document_t* doc) {
    if (doc->data != NULL) {
        free(doc->data);
    }
    if (doc->lines != NULL) {
        free(doc->lines);
    }
    free(doc);
}
