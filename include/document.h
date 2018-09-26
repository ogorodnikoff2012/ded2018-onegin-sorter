#pragma once

#include <stdbool.h>
#include <stdint.h>

/** @file */

typedef char * line_t;

/** Text file separated into lines by `'\\n'` symbol */
typedef struct {
    char* data; ///< Raw data from file
    int data_size; ///< Raw data size; used for internal purposes
    line_t* lines; ///< Array of lines of text
    int lines_cnt; ///< Length of `lines` array
} document_t;

/** Open file `filename` and read it
 * \return pointer to the document if no errors occured or NULL otherwise */
document_t* read_document(const char* filename);

/** Check if the document is correct utf-8 file
 * \param [out] err_pos If file is malformed, position of the first illegal byte is written here
 * */
bool check_document(const document_t* document, int* err_pos);

/** Write lines of the document to the file `filename`
 * \return `true` if succeded and `false` otherwise
 * */
bool print_document(const document_t* document, const char* filename);

/** Free al structures related to the document
 * */
void close_document(document_t* document);

/** Get symbol by its position in original file */
int32_t symbol_at(const document_t* document, int pos);
