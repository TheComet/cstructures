#include "cstructures/string.h"
#include "cstructures/vector.h"
#include "cstructures/memory.h"
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
enum string_status_e
string_create(struct string_t** str)
{
    enum string_status_e status;

    *str = MALLOC(sizeof **str);
    if (*str == NULL)
        goto malloc_failed;

    if ((status = string_init(*str)) != STR_OK)
        goto init_failed;

    return STR_OK;

    init_failed   : FREE(*str);
    malloc_failed : return STR_OOM;
}

/* ------------------------------------------------------------------------- */
enum string_status_e
string_init(struct string_t* str)
{
    if (vector_init(&str->buf, sizeof(char)) != VEC_OK)
        return STR_OOM;
    return STR_OK;
}

/* ------------------------------------------------------------------------- */
void
string_deinit(struct string_t* str)
{
    vector_deinit(&str->buf);
}

/* ------------------------------------------------------------------------- */
void
string_destroy(struct string_t* str)
{
    string_deinit(str);
    FREE(str);
}

/* ------------------------------------------------------------------------- */
int
string_getline(struct string_t* str, FILE* fp)
{
    int ret;
    assert(fp);

    vector_clear(&str->buf);
    while ((ret = fgetc(fp)) != EOF)
    {
        char c = (char)ret;

        /* Ignore carriage */
        if (c == '\r')
            continue;

        /* If we encounter a newline, complete string and return */
        if (c == '\n')
        {
            /* empty line? */
            if (vector_count(&str->buf) == 0)
                continue;

            char nullterm = '\0';
            if (vector_push(&str->buf, &nullterm) != VEC_OK)
                return -1;
            return 1;
        }

        if (vector_push(&str->buf, &c) != VEC_OK)
            return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
char*
string_tok(struct string_t* str, char delimiter, char** saveptr)
{
    char* begin_ptr;
    char* end_ptr;

    if(str)
        *saveptr = (char*)vector_data(&str->buf) - 1;

    /* no more tokens */
    if(!*saveptr)
        return NULL;

    /* get first occurrence of token in string */
    begin_ptr = *saveptr + 1;
    end_ptr = (char*)strchr(begin_ptr, delimiter);
    if(!end_ptr)
        *saveptr = NULL; /* last token has been reached */
    else
    {
        /* update saveptr and replace delimiter with null terminator */
        *saveptr = end_ptr;
        **saveptr = '\0';
    }

    /* empty tokens */
    if(*begin_ptr == '\0')
        return NULL;
    return begin_ptr;
}
