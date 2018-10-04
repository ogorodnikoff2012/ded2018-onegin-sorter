#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "document.h"
#include "unicode.h"
#define DATA_EMPTY_SUFFIX 1 // Needed for zero-byte terminator

static void separate_into_lines(document_t* doc) {
    char* data_begin = doc->data;
    char* data_end = data_begin + doc->data_size;
    char* last_line_begin = data_begin;
    doc->lines_cnt = 0;
    for (char* it = data_begin; it != data_end; ++it) {
        if (iscntrl(*it)) {
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

/* Some unicode documents have Byte Order Mark in the beginning. In UTF-8 it is used just
 * for compability with UTF16 and UTF32. Let's remove it! */
static void remove_bom(document_t* doc) {
    /* BOM : EF BB BF */
    if (doc->data_size >= 3 + DATA_EMPTY_SUFFIX &&
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

    /* Allocate and initialize document fields */
    document_t* doc = calloc(sizeof(document_t), 1);
    doc->data_size = filesize + DATA_EMPTY_SUFFIX;
    doc->data = malloc(doc->data_size);
    doc->data[filesize] = doc->data[0] = '\0';

    if (read(fd, doc->data, filesize) == -1) {
        close_document(doc);
        return NULL;
    }
    close(fd);

    remove_bom(doc);
    separate_into_lines(doc);

    return doc;
}

bool check_document(const document_t* doc, int* err_pos) {
    const char* file_begin = doc->data;
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
    const char* iter = doc->data + pos;
    return next_symbol(&iter);
}

/* To decrease number of write() syscalls, let's use buffer. Standard fwrite()
 * uses 8 KiB buffer. It's quite small. */
#ifndef BUFFERED_WRITE_BUFFER_SIZE
#define BUFFERED_WRITE_BUFFER_SIZE (1<<20) /* 1 MiB */
#endif

typedef struct {
    char data[BUFFERED_WRITE_BUFFER_SIZE];
    int used;
} buffer_t;

static inline int write_to_buffer(buffer_t* buffer, const char* src, int len) {
    int write_len = BUFFERED_WRITE_BUFFER_SIZE - buffer->used;
    if (len < write_len) {
        write_len = len;
    }
    memcpy(buffer->data + buffer->used, src, write_len);
    buffer->used += write_len;
    return write_len;
}

static inline int flush_buffer(buffer_t* buffer, int fd) {
    int result = write(fd, buffer->data, buffer->used);
    if (result != -1) {
        buffer->used = 0;
    }
    return result;
}

static inline bool buffered_write(int fd, buffer_t* buffer, const char* str, int len) {
    while (len > 0) {
        int written = write_to_buffer(buffer, str, len);
        str += written;
        len -= written;
        if (buffer->used == BUFFERED_WRITE_BUFFER_SIZE) {
            int result = flush_buffer(buffer, fd);
            if (result == -1) {
                return false;
            }
        }
    }
    return true;
}

bool print_document(const document_t* doc, const char* filename) {
    int fd = creat(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        return false;
    }

    buffer_t buf;
    buf.used = 0;

    for (int i = 0; i < doc->lines_cnt; ++i) {
        if (!buffered_write(fd, &buf, doc->lines[i], strlen(doc->lines[i])) ||
            !buffered_write(fd, &buf, "\n", 1)) {
            close(fd);
            return false;
        }
    }
    flush_buffer(&buf, fd);
    close(fd);
    return true;
}

void close_document(document_t* doc) {
    if (doc == NULL) {
        return;
    }
    if (doc->data != NULL) {
        free(doc->data);
    }
    if (doc->lines != NULL) {
        free(doc->lines);
    }
    free(doc);
}
