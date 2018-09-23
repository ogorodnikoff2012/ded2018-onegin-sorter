#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "document.h"

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
    doc->data_size = filesize + 2;
    doc->data = malloc(doc->data_size);
    doc->data[filesize + 1] = doc->data[0] = '\0';
    if (read(fd, doc->data + 1, filesize) == -1) {
        close_document(doc);
        return NULL;
    }
    close(fd);

    separate_into_lines(doc);

    return doc;
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
