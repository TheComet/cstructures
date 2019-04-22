#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "cstructures/vector.h"
#include "cstructures/memory.h"

#define VEC_INVALID_INDEX (vec_size_t)-1

#define VECTOR_NEEDS_REALLOC(x) \
        ((x)->count == (x)->capacity)

/* ----------------------------------------------------------------------------
 * Static functions
 * ------------------------------------------------------------------------- */

/*!
 * @brief Expands the underlying memory.
 *
 * This implementation will expand the memory by a factor of 2 each time this
 * is called. All elements are copied into the new section of memory.
 * @param[in] insertion_index Set to VEC_INVALID_INDEX if (no space should be
 * made for element insertion. Otherwise this parameter specifies the index of
 * the element to "evade" when re-allocating all other elements.
 * @param[in] new_count The number of elements to allocate memory for.
 * @note No checks are performed to make sure the target size is large enough.
 */
static enum vec_status_e
vector_realloc(struct vec_t *vector,
              vec_size_t insertion_index,
              vec_size_t new_count);

/* ----------------------------------------------------------------------------
 * Exported functions
 * ------------------------------------------------------------------------- */
enum vec_status_e
vector_create(struct vec_t** vector, const vec_size_t element_size)
{
    if (!(*vector = MALLOC(sizeof **vector)))
        return VEC_OOM;
    vector_init(*vector, element_size);
    return VEC_OK;
}

/* ------------------------------------------------------------------------- */
enum vec_status_e
vector_init(struct vec_t* vector, const vec_size_t element_size)
{
    assert(vector);
    memset(vector, 0, sizeof *vector);
    vector->element_size = element_size;
    return VEC_OK;
}

/* ------------------------------------------------------------------------- */
void
vector_deinit(struct vec_t* vector)
{
    assert(vector);

    XFREE(vector->data);
}

/* ------------------------------------------------------------------------- */
void
vector_free(struct vec_t* vector)
{
    assert(vector);
    vector_deinit(vector);
    FREE(vector);
}

/* ------------------------------------------------------------------------- */
void
vector_clear(struct vec_t* vector)
{
    assert(vector);
    /*
     * No need to free or overwrite existing memory, just reset the counter
     * and let future insertions overwrite
     */
    vector->count = 0;
}

/* ------------------------------------------------------------------------- */
void
vector_compact(struct vec_t* vector)
{
    assert(vector);

    if (vector->count == 0)
    {
        XFREE(vector->data);
        vector->data = NULL;
        vector->capacity = 0;
    }
    else
    {
        /* If this fails (realloc shouldn't fail when specifying a smaller size
         * but who knows) it doesn't really matter. The vector will be in an
         * unchanged state and functionally still be identical */
        vector_realloc(vector, VEC_INVALID_INDEX, vector_count(vector));
    }
}

/* ------------------------------------------------------------------------- */
enum vec_status_e
vector_reserve(struct vec_t* vector, vec_size_t size)
{
    assert(vector);

    if (vector->capacity < size)
    {
        enum vec_status_e result;
        if ((result = vector_realloc(vector, VEC_INVALID_INDEX, size)) != VEC_OK)
            return result;
    }

    return VEC_OK;
}

/* ------------------------------------------------------------------------- */
enum vec_status_e
vector_resize(struct vec_t* vector, vec_size_t size)
{
    enum vec_status_e result;
    assert(vector);

    if ((result = vector_reserve(vector, size)) != VEC_OK)
        return result;

    vector->count = size;

    return VEC_OK;
}

/* ------------------------------------------------------------------------- */
enum vec_status_e
vector_emplace(struct vec_t* vector, void** emplaced)
{
    enum vec_status_e status;
    assert(vector);

    if (VECTOR_NEEDS_REALLOC(vector))
        if ((status = vector_realloc(vector,
                                     VEC_INVALID_INDEX,
                                     vector_count(vector) * CSTRUCTURES_VEC_EXPAND_FACTOR)) != VEC_OK)
            return status;

    *emplaced = vector->data + (vector->element_size * vector->count);
    ++(vector->count);

    return VEC_OK;
}

/* ------------------------------------------------------------------------- */
enum vec_status_e
vector_push(struct vec_t* vector, const void* data)
{
    void* emplaced;
    enum vec_status_e status;

    assert(vector);
    assert(data);

    if ((status = vector_emplace(vector, &emplaced)) != VEC_OK)
        return status;
    memcpy(emplaced, data, vector->element_size);
    return VEC_OK;
}

/* ------------------------------------------------------------------------- */
enum vec_status_e
vector_push_vector(struct vec_t* vector, const struct vec_t* source_vector)
{
    enum vec_status_e result;

    assert(vector);
    assert(source_vector);

    /* make sure element sizes are equal */
    if (vector->element_size != source_vector->element_size)
        return VEC_DIFFERENT_ELEMENT_SIZES;

    /* make sure there's enough space in the target vector */
    if (vector->count + source_vector->count > vector->capacity)
        if ((result = vector_realloc(vector, VEC_INVALID_INDEX, vector->count + source_vector->count)) != VEC_OK)
            return result;

    /* copy data */
    memcpy(vector->data + (vector->count * vector->element_size),
           source_vector->data,
           source_vector->count * vector->element_size);
    vector->count += source_vector->count;

    return VEC_OK;
}

/* ------------------------------------------------------------------------- */
void*
vector_pop(struct vec_t* vector)
{
    assert(vector);

    if (!vector->count)
        return NULL;

    --(vector->count);
    return vector->data + (vector->element_size * vector->count);
}

/* ------------------------------------------------------------------------- */
void*
vector_back(const struct vec_t* vector)
{
    assert(vector);

    if (!vector->count)
        return NULL;

    return vector->data + (vector->element_size * (vector->count - 1));
}

/* ------------------------------------------------------------------------- */
enum vec_status_e
vector_insert_emplace(struct vec_t* vector, vec_size_t index, void** emplaced)
{
    vec_size_t offset;
    enum vec_status_e status;

    assert(vector);

    /*
     * Normally the last valid index is (capacity-1), but in this case it's valid
     * because it's possible the user will want to insert at the very end of
     * the vector.
     */
    assert(index <= vector->count);

    /* re-allocate? */
    if (vector->count == vector->capacity)
    {
        if ((status = vector_realloc(vector,
                                     index,
                                     vector_count(vector) * CSTRUCTURES_VEC_EXPAND_FACTOR)) != VEC_OK)
            return status;
    }
    else
    {
        /* shift all elements up by one to make space for insertion */
        vec_size_t total_size = vector->count * vector->element_size;
        offset = vector->element_size * index;
        memmove((void*)((uintptr_t)vector->data + offset + vector->element_size),
                (void*)((uintptr_t)vector->data + offset),
                total_size - offset);
    }

    /* element is inserted */
    ++vector->count;

    /* return pointer to memory of new element */
    *emplaced = (void*)(vector->data + index * vector->element_size);

    return VEC_OK;
}

/* ------------------------------------------------------------------------- */
enum vec_status_e
vector_insert(struct vec_t* vector, vec_size_t index, void* data)
{
    void* emplaced;
    enum vec_status_e status;

    assert(vector);
    assert(data);

    if ((status = vector_insert_emplace(vector, index, &emplaced)) != VEC_OK)
        return status;
    memcpy(emplaced, data, vector->element_size);
    return VEC_OK;
}

/* ------------------------------------------------------------------------- */
void
vector_erase_index(struct vec_t* vector, vec_size_t index)
{
    assert(vector);

    if (index >= vector->count)
        return;

    if (index == vector->count - 1)
        /* last element doesn't require memory shifting, just pop it */
        vector_pop(vector);
    else
    {
        /* shift memory right after the specified element down by one element */
        vec_size_t offset = vector->element_size * index;  /* offset to the element being erased in bytes */
        vec_size_t total_size = vector->element_size * vector->count; /* total current size in bytes */
        memmove((void*)((uintptr_t)vector->data + offset),   /* target is to overwrite the element specified by index */
                (void*)((uintptr_t)vector->data + offset + vector->element_size),    /* copy beginning from one element ahead of element to be erased */
                total_size - offset - vector->element_size);     /* copying number of elements after element to be erased */
        --vector->count;
    }
}

/* ------------------------------------------------------------------------- */
void
vector_erase_element(struct vec_t* vector, void* element)
{
    uintptr_t last_element;

    assert(vector);
    last_element = (uintptr_t)vector->data + (vector->count-1) * vector->element_size;
    assert(element);
    assert((uintptr_t)element >= (uintptr_t)vector->data);
    assert((uintptr_t)element <= (uintptr_t)last_element);

    if (element != (void*)last_element)
    {
        memmove(element,    /* target is to overwrite the element */
                (void*)((uintptr_t)element + vector->element_size), /* read everything from next element */
                last_element - (uintptr_t)element);
    }
    --vector->count;
}

/* ------------------------------------------------------------------------- */
void*
vector_get_element(const struct vec_t* vector, vec_size_t index)
{
    assert(vector);

    if (index >= vector->count)
        return NULL;
    return vector->data + (vector->element_size * index);
}

/* ----------------------------------------------------------------------------
 * Static functions
 * ------------------------------------------------------------------------- */
static enum vec_status_e
vector_realloc(struct vec_t *vector,
              vec_size_t insertion_index,
              vec_size_t new_capacity)
{
    void* new_data;

    /*
     * If vector hasn't allocated anything yet, just allocated the requested
     * amount of memory and return immediately.
     */
    if (!vector->data)
    {
        new_capacity = (new_capacity == 0 ? CSTRUCTURES_VEC_MIN_CAPACITY : new_capacity);
        vector->data = MALLOC(new_capacity * vector->element_size);
        if (!vector->data)
            return VEC_OOM;
        vector->capacity = new_capacity;
        return VEC_OK;
    }

    /* prepare for reallocating data */
    if ((new_data = REALLOC(vector->data, new_capacity * vector->element_size)) == NULL)
        return VEC_OOM;
    vector->data = new_data;

    /* if (no insertion index is required, copy all data to new memory */
    if (insertion_index != VEC_INVALID_INDEX)
    {
        void* old_upper_elements = vector->data + (insertion_index + 0) * vector->element_size;
        void* new_upper_elements = vector->data + (insertion_index + 1) * vector->element_size;
        vec_size_t upper_element_count = vector->capacity - insertion_index;
        memmove(new_upper_elements, old_upper_elements, upper_element_count * vector->element_size);
    }

    vector->capacity = new_capacity;

    return VEC_OK;
}
