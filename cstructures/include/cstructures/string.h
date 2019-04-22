#ifndef CSTRUCTURES_STRING_H
#define CSTRUCTURES_STRING_H

#include "cstructures/config.h"
#include "cstructures/vector.h"
#include <stdint.h>
#include <stdio.h>

C_BEGIN

enum string_status_e
{
    STR_OK  = 0,
    STR_OOM = -1
};

struct string_t
{
    struct vec_t buf;
};

struct string_split_state_t
{
    char token;
    size_t split_count;
};

CSTRUCTURES_PUBLIC_API enum string_status_e
string_create(struct string_t** str);

CSTRUCTURES_PUBLIC_API enum string_status_e
string_init(struct string_t* str);

CSTRUCTURES_PUBLIC_API void
string_deinit(struct string_t* str);

CSTRUCTURES_PUBLIC_API void
string_destroy(struct string_t* str);

/*!
 * @brief Reads a line of text from the stream.
 * @note Empty lines are ignored, i.e. this function is guaranteed to return
 * a non-zero length string.
 * @return Returns 1 if a new string was read successfully. Returns 0 if there
 * are no more strings to read. Returns -1 if an error occurred.
 */
CSTRUCTURES_PUBLIC_API int
string_getline(struct string_t* str, FILE* fp);

char*
string_tok(struct string_t* str, char delimiter, char** saveptr);

#define string_length(str) \
        (vector_count(&str->buf) - 1)

#define string_cstr(str) \
        ((char*)vector_data(&(str)->buf))

C_END

#endif  /* CSTRUCTURES_STRING_H */
