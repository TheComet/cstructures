#include "cstructures/btree.h"
#include "cstructures/memory.h"
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
static enum btree_status_e
btree_realloc(struct btree_t* btree, btree_size_t insertion_index, btree_size_t new_capacity)
{
    /* clamp to minimum configured capacity */
    if (new_capacity < CSTRUCTURES_BTREE_MIN_CAPACITY)
        new_capacity = CSTRUCTURES_BTREE_MIN_CAPACITY;

    /*
     * If btree hasn't allocated anything yet, just allocated the requested
     * amount of memory and return immediately.
     */
    if (!btree->data)
    {
        btree->data = MALLOC(new_capacity * BTREE_KV_SIZE(btree));
        if (!btree->data)
            return BTREE_OOM;
        btree->capacity = new_capacity;
        return BTREE_OK;
    }

    /*
     * If the new capacity is larger than the old capacity, then we realloc
     * before shifting around the data.
     */
    if (new_capacity >= btree->capacity)
    {
        void* new_data = REALLOC(btree->data, new_capacity * BTREE_KV_SIZE(btree));
        if (!new_data)
            return BTREE_OOM;
        btree->data = new_data;
    }

    /*
     * The keys are correctly placed in memory, but now that the capacity has
     * grown, the values need to be moved forwards in the data buffer
     */
    if (insertion_index == BTREE_INVALID_KEY)
    {
        btree_size_t old_capacity = btree->capacity;
        void* old_values = BTREE_VALUE_BEG_CAP(btree, old_capacity);
        void* new_values = BTREE_VALUE_BEG_CAP(btree, new_capacity);
        memmove(new_values, old_values, old_capacity * btree->value_size);
    }
    else
    {
        btree_size_t old_capacity = btree->capacity;
        void* old_lower_values = BTREE_VALUE_BEG_CAP(btree, old_capacity);
        void* new_lower_values = BTREE_VALUE_BEG_CAP(btree, new_capacity);
        void* old_upper_values = BTREE_VALUE_CAP(btree, insertion_index, old_capacity);
        void* new_upper_values = BTREE_VALUE_CAP(btree, insertion_index + 1, new_capacity);
        btree_size_t lower_values_to_move = insertion_index;
        btree_size_t upper_values_to_move = old_capacity - insertion_index;
        btree_key_t* old_upper_keys = BTREE_KEY(btree, insertion_index);
        btree_key_t* new_upper_keys = BTREE_KEY(btree, insertion_index + 1);
        btree_size_t keys_to_move = old_capacity - insertion_index;

        memmove(new_upper_values, old_upper_values, upper_values_to_move * btree->value_size);
        memmove(new_lower_values, old_lower_values, lower_values_to_move * btree->value_size);
        memmove(new_upper_keys, old_upper_keys, keys_to_move);
    }

    /*
     * If the new capacity is smaller than the old capacity, we have to realloc
     * after moving around the data as to not read from memory out of bounds
     * of the buffer.
     */
    if (new_capacity < btree->capacity)
    {
        void* new_data = REALLOC(btree->data, new_capacity * BTREE_KV_SIZE(btree));
        if (new_data)
            btree->data = new_data;
        else
        {
            /*
             * This should really never happen, but if the realloc to a smaller
             * size fails, the btree will be in a consistent state if the
             * capacity is updated to the new capacity.
             */
        }
    }

    btree->capacity = new_capacity;

    return BTREE_OK;
}

/* ------------------------------------------------------------------------- */
enum btree_status_e
btree_create(struct btree_t** btree, uint32_t value_size)
{
    *btree = MALLOC(sizeof **btree);
    if (*btree == NULL)
        return BTREE_OOM;
    btree_init(*btree, value_size);
    return BTREE_OK;
}

/* ------------------------------------------------------------------------- */
enum btree_status_e
btree_init(struct btree_t* btree, uint32_t value_size)
{
    assert(btree);
    btree->data = NULL;
    btree_count(btree) = 0;
    btree->capacity = 0;
    btree->value_size = value_size;

    return BTREE_OK;
}

/* ------------------------------------------------------------------------- */
void btree_deinit(struct btree_t* btree)
{
    assert(btree);
    btree_clear(btree);
    btree_compact(btree);
}

/* ------------------------------------------------------------------------- */
void
btree_free(struct btree_t* btree)
{
    assert(btree);
    btree_deinit(btree);
    FREE(btree);
}

/* ------------------------------------------------------------------------- */
/* algorithm taken from GNU GCC stdlibc++'s lower_bound function, line 2121 in stl_algo.h */
/* https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a02014.html */
/*
 * 1) If the key exists, then a pointer to that key is returned.
 * 2) If the key does not exist, then the first valid key who's value is less
 *    than the key being searched for is returned.
 * 3) If there is no key who's value is less than the searched-for key, the
 *    returned pointer will point to the address after the last valid key in
 *    the array.
 */
static btree_key_t*
btree_find_lower_bound(const struct btree_t* btree, btree_key_t key)
{
    btree_size_t half;
    btree_key_t* middle;
    btree_key_t* found;
    btree_size_t len;

    assert(btree);

    found = BTREE_KEY(btree, 0);  /* start search at key index 0 */
    len = btree_count(btree);

    while (len > 0)
    {
        half = len >> 1;
        middle = found + half;
        if (*middle < key)
        {
            found = middle;
            ++found;
            len = len - half - 1;
        }
        else
            len = half;
    }

    return found;
}

/* ------------------------------------------------------------------------- */
enum btree_status_e
btree_insert_new(struct btree_t* btree, btree_key_t key, const void* value)
{
    btree_key_t* lower_bound;
    btree_size_t insertion_index;
    enum btree_status_e status;

    assert(btree);

    /* lookup location in btree to insert */
    lower_bound = btree_find_lower_bound(btree, key);
    if (lower_bound < BTREE_KEY_END(btree) && *lower_bound == key)
        return BTREE_EXISTS;
    insertion_index = BTREE_KEY_TO_IDX(btree, lower_bound);

    /* May need to realloc */
    if (BTREE_NEEDS_REALLOC(btree))
    {
        if ((status = btree_realloc(btree, BTREE_INVALID_KEY, btree->capacity * CSTRUCTURES_BTREE_EXPAND_FACTOR)) != BTREE_OK)
            return status;
    }
    else
    {
        btree_size_t entries_to_move = btree_count(btree) - insertion_index;
        memmove(lower_bound + 1, lower_bound, entries_to_move * sizeof(btree_key_t));
        memmove(BTREE_VALUE(btree, insertion_index + 1), BTREE_VALUE(btree, insertion_index), entries_to_move * btree->value_size);
    }

    memcpy(BTREE_KEY(btree, insertion_index), &key, sizeof(btree_key_t));
    if (btree->value_size)
        memcpy(BTREE_VALUE(btree, insertion_index), value, btree->value_size);
    btree->count++;

    return BTREE_OK;
}

/* ------------------------------------------------------------------------- */
enum btree_status_e
btree_set_existing(struct btree_t* btree, btree_key_t key, const void* value)
{
    void* found;
    assert(btree);
    assert(btree->value_size > 0);
    assert(value);

    if (btree->value_size == 0)
        return BTREE_OK;

    if ((found = btree_find(btree, key)) == NULL)
        return BTREE_NOT_FOUND;

    memcpy(found, value, btree->value_size);
    return BTREE_OK;
}

/* ------------------------------------------------------------------------- */
enum btree_status_e
btree_set_or_insert(struct btree_t* btree, btree_key_t key, const void* value)
{
    btree_key_t* lower_bound;
    btree_size_t insertion_index;
    enum btree_status_e status;

    assert(btree);
    assert(btree->value_size > 0);
    assert(value);

    /* lookup location in btree to insert */
    lower_bound = btree_find_lower_bound(btree, key);
    insertion_index = BTREE_KEY_TO_IDX(btree, lower_bound);
    if (lower_bound < BTREE_KEY_END(btree) && *lower_bound == key)
    {
        memcpy(BTREE_VALUE(btree, insertion_index), value, btree->value_size);
        return BTREE_OK;
    }

    /* May need to realloc */
    if (BTREE_NEEDS_REALLOC(btree))
        if ((status = btree_realloc(btree, BTREE_INVALID_KEY, btree->capacity * CSTRUCTURES_BTREE_EXPAND_FACTOR)) != BTREE_OK)
            return status;

    memcpy(BTREE_KEY(btree, insertion_index), &key, sizeof(btree_key_t));
    memcpy(BTREE_VALUE(btree, insertion_index), value, btree->value_size);
    btree->count++;

    return BTREE_OK;
}

/* ------------------------------------------------------------------------- */
void*
btree_find(const struct btree_t* btree, btree_key_t key)
{
    btree_key_t* lower_bound;
    btree_size_t idx;

    assert(btree);
    assert(btree->value_size > 0);

    lower_bound = btree_find_lower_bound(btree, key);
    if (lower_bound >= BTREE_KEY_END(btree) || *lower_bound != key)
        return NULL;

    idx = BTREE_KEY_TO_IDX(btree, lower_bound);
    return BTREE_VALUE(btree, idx);
}

/* ------------------------------------------------------------------------- */
static btree_size_t
btree_find_index_of_matching_value(const struct btree_t* btree, const void* value)
{
    void* current_value;
    btree_size_t i;

    for (i = 0, current_value = BTREE_VALUE_BEG(btree);
         i != btree_count(btree);
        ++i, current_value = BTREE_VALUE(btree, i))
    {
        if (memcmp(current_value, value, btree->value_size) == 0)
            return i;
    }

    return (btree_size_t)-1;
}

/* ------------------------------------------------------------------------- */
btree_key_t*
btree_find_key(const struct btree_t* btree, const void* value)
{
    btree_size_t i;

    assert(btree);
    assert(btree->value_size > 0);
    assert(value);

    if ((i = btree_find_index_of_matching_value(btree, value)) == (btree_size_t)-1)
        return NULL;

    return BTREE_KEY(btree, i);
}

/* ------------------------------------------------------------------------- */
int
btree_find_and_compare(const struct btree_t* btree, btree_key_t key, const void* value)
{
    void* inserted_value;

    assert(btree);
    assert(btree->value_size > 0);
    assert(value);

    inserted_value = btree_find(btree, key);
    if (inserted_value == NULL)
        return 0;

    return memcmp(inserted_value, value, btree->value_size) == 0;
}

/* ------------------------------------------------------------------------- */
void*
btree_get_any_value(const struct btree_t* btree)
{
    assert(btree);
    assert(btree->value_size > 0);

    if (btree_count(btree) == 0)
        return NULL;
    return BTREE_VALUE(btree, 0);
}

/* ------------------------------------------------------------------------- */
int
btree_key_exists(struct btree_t* btree, btree_key_t key)
{
    btree_key_t* lower_bound;

    assert(btree);

    lower_bound = btree_find_lower_bound(btree, key);
    if (lower_bound < BTREE_KEY_END(btree) && *lower_bound == key)
        return 1;
    return 0;
}

/* ------------------------------------------------------------------------- */
btree_key_t
btree_find_unused_key(struct btree_t* btree)
{
    btree_key_t key = 0;

    assert(btree);

    if (btree->data)
        while (*BTREE_KEY(btree, key) == key)
            key++;

    return key;
}

/* ------------------------------------------------------------------------- */
btree_key_t
btree_erase_index(struct btree_t* btree, btree_size_t idx)
{
    btree_key_t* lower_bound;
    btree_key_t key;
    btree_size_t entries_to_move;

    lower_bound = BTREE_KEY(btree, idx);
    key = *lower_bound;
    entries_to_move = btree_count(btree) - idx;
    memmove(lower_bound, lower_bound+1, entries_to_move * sizeof(btree_key_t));
    memmove(BTREE_VALUE(btree, idx), BTREE_VALUE(btree, idx+1), entries_to_move * btree->value_size);
    btree->count--;

    return key;
}

/* ------------------------------------------------------------------------- */
enum btree_status_e
btree_erase(struct btree_t* btree, btree_key_t key)
{
    btree_key_t* lower_bound;

    assert(btree);

    lower_bound = btree_find_lower_bound(btree, key);
    if (lower_bound >= BTREE_KEY_END(btree) || *lower_bound != key)
        return BTREE_NOT_FOUND;

    btree_erase_index(btree, BTREE_KEY_TO_IDX(btree, lower_bound));

    return BTREE_OK;
}

/* ------------------------------------------------------------------------- */
btree_key_t
btree_erase_value(struct btree_t* btree, const void* value)
{
    btree_size_t idx;

    assert(btree);
    assert(btree->value_size > 0);
    assert(value);

    idx = btree_find_index_of_matching_value(btree, value);
    if (idx == (btree_size_t)-1)
        return BTREE_INVALID_KEY;

    return btree_erase_index(btree, idx);
}

/* ------------------------------------------------------------------------- */
btree_key_t
btree_erase_internal_value(struct btree_t* btree, const void* value)
{
    btree_size_t idx;

    assert(btree);
    assert(btree->value_size > 0);
    assert(value);
    assert(BTREE_VALUE_BEG(btree) <= value);
    assert(value < BTREE_VALUE_END(btree));

    idx = BTREE_VALUE_TO_IDX(btree, value);
    return btree_erase_index(btree, idx);
}

/* ------------------------------------------------------------------------- */
void
btree_clear(struct btree_t* btree)
{
    assert(btree);

    btree->count = 0;
}

/* ------------------------------------------------------------------------- */
void
btree_compact(struct btree_t* btree)
{
    assert(btree);

    if (btree_count(btree) == 0)
    {
        XFREE(btree->data);
        btree->data = NULL;
        btree->capacity = 0;
    }
    else
    {
        btree_realloc(btree, BTREE_INVALID_KEY, btree_count(btree));
    }
}
