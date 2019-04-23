#ifndef CSTRUCTURES_BTREE_H
#define CSTRUCTURES_BTREE_H

#include "cstructures/config.h"
#include "cstructures/hash.h"

C_BEGIN

#if defined(CSTRUCTURES_BTREE_64BIT_KEYS)
typedef uint64_t btree_key_t;
#else
typedef uint32_t btree_key_t;
#endif

#if defined(CSTRUCTURES_BTREE_64BIT_CAPACITY)
typedef uint64_t btree_size_t;
#else
typedef uint32_t btree_size_t;
#endif

/* Memory address of the last valid key + 1 (i.e. dereferencing this is not a valid key) */
#define BTREE_KEY_END(btree) \
        BTREE_KEY(btree, btree_count(btree))

/* Memory address of the first value assuming the specified capacity */
#define BTREE_VALUE_BEG_CAP(btree, cap) \
        (void*)((btree_key_t*)(btree)->data + (cap))

/* Memory address of the first value */
#define BTREE_VALUE_BEG(btree) \
        BTREE_VALUE_BEG_CAP(btree, (btree)->capacity)

/* Memory address of the last valid value + 1 (i.e. dereferencing this is not a valid value) */
#define BTREE_VALUE_END(btree) \
        BTREE_VALUE(btree, btree_count(btree))

/* Memory address of key at index i */
#define BTREE_KEY(btree, i) \
        ((btree_key_t*)(btree)->data + (i))

/* Memory address of value at index i */
#define BTREE_VALUE(btree, i) \
        (void*)((uint8_t*)BTREE_VALUE_BEG(btree) + (btree)->value_size * (i))

#define BTREE_VALUE_CAP(btree, i, cap) \
        (void*)((uint8_t*)BTREE_VALUE_BEG_CAP(btree, cap) + (btree)->value_size * (i))

/* Convert a key memory address to an index */
#define BTREE_KEY_TO_IDX(btree, key) \
        (btree_size_t)((btree_key_t*)(key) - (btree_key_t*)(btree)->data)

/* Convert a value memory address to an index */
#define BTREE_VALUE_TO_IDX(btree, value) \
        (btree_size_t)(((uint8_t*)value - (uint8_t*)BTREE_VALUE_BEG(btree)) / btree_value_size(btree))

#define BTREE_KV_SIZE(btree) \
        (sizeof(btree_key_t) + (btree)->value_size)

#define BTREE_NEEDS_REALLOC(btree) \
        ((btree)->count == (btree)->capacity)

#define BTREE_INVALID_KEY ((btree_key_t)-1)

enum btree_status_e
{
    BTREE_NOT_FOUND    = 2,
    BTREE_EXISTS       = 1,
    BTREE_OK           = 0,
    BTREE_OOM          = -1
};

/*!
 * @brief Implements a container of sorted key-value pairs stored in flattened
 * memory (sorted by key).
 */
struct btree_t
{
    void* data;
    btree_size_t count;
    btree_size_t capacity;
    uint32_t value_size;
};

/*!
 * @brief Creates a new btree object on the heap.
 * @param[out] btree Pointer to the new object is written to this parameter.
 * @param[in] value_size The size in bytes of the values that will be stored.
 * You have the choice to either copy values in-place, in which case you can
 * specify sizeof(my_type_t), or, if you are working with very large value types
 * or with different-sized values (such as strings) you can specify sizeof(void*)
 * and store pointers to those objects instead. In the former case, you don't
 * have to worry about having to explicitly free() the data you store. In the
 * latter case, the btree only holds references to data but you are responsible
 * for managing the lifetime of each value.
 * @return Returns the newly created btree object. It must be freed with
 * btree_free() when no longer required.
 */
CSTRUCTURES_PUBLIC_API enum btree_status_e
btree_create(struct btree_t** btree, uint32_t value_size);

/*!
 * @brief Initialises an existing btree object.
 * @note This does **not** free existing items if they aren't in-place. If
 * you have inserted pointers to objects into your btree and call this, those
 * items will be lost and a memory leak will have been created.
 * @param[in] btree The btree object to initialise.
 */
CSTRUCTURES_PUBLIC_API enum btree_status_e
btree_init(struct btree_t* btree, uint32_t value_size);

CSTRUCTURES_PUBLIC_API void
btree_deinit(struct btree_t* btree);

/*!
 * @brief Destroys an existing btree object and FREEs the underlying memory.
 * @note Elements inserted into the btree are not FREEd.
 * @param[in] btree The btree object to free.
 */
CSTRUCTURES_PUBLIC_API void
btree_free(struct btree_t* btree);

/*!
 * @brief Inserts an item into the btree using a key.
 *
 * @note Complexity is O(log2(n)) to find the insertion point.
 *
 * @param[in] btree The btree object to insert into.
 * @param[in] key A unique key to assign to the item being inserted. The
 * key must not exist in the btree, or the item will not be inserted.
 * @param[in] value A pointer to the data to insert into the tree. The data
 * pointed to is copied into the structure. If you are storing pointers to
 * strings (having specified sizeof(char*) for value_size), you would pass
 * a double-pointer to the string for the pointer to be copied into the btree.
 * @return Returns BTREE_OK if insertion was successful. Returns BTREE_EXISTS
 * if the key already exists (in which case nothing is inserted). Returns
 * BTREE_OOM if not enough memory was available in the case of a reallocation.
 */
CSTRUCTURES_PUBLIC_API enum btree_status_e
btree_insert_new(struct btree_t* btree, btree_key_t key, const void* value);

/*!
 * @brief Updates an existing value. If the key doesn't exist, this function
 * does nothing.
 * @note This is a convenience function that uses btree_find() to obtain a
 * pointer to the value and memcpy'ing the new value into its place.
 * @param[in] btree A pointer to the btree object to change the value of.
 * @param[in] key The unique key associated with the value you want to change.
 * @param[in] value The new value to set.
 * @return Returns BTREE_OK if the value was found and updated. Returns
 * BTREE_NOT_FOUND if the key was not found.
 */
CSTRUCTURES_PUBLIC_API enum btree_status_e
btree_set_existing(struct btree_t* btree, btree_key_t key, const void* value);

/*!
 * @brief Either inserts a new value if it doesn't exist, or updates an
 * existing value if the key does exist.
 * @param[in] btree The btree to insert into.
 * @param[in] key The unique key associated with the value you want to set.
 * @param[in] value A pointer to the data to insert into the tree. The data
 * must be at least btree_value_size() in bytes. This is set during btree
 * creation.
 * @return Returns BTREE_OK on success. Returns BTREE_OOM if not enough memory
 * was available in the case of a reallocation.
 */
CSTRUCTURES_PUBLIC_API enum btree_status_e
btree_set_or_insert(struct btree_t* btree, btree_key_t key, const void* value);

/*!
 * @brief Looks for the specified key in the btree and returns a pointer to the
 * value in the structure. This is useful if you need to store data directly in
 * the memory occupied by the pointer and wish to modify it.
 * @note Complexity is O(log2(n))
 * @warning The returned pointer can be invalidated if any insertions or deletions
 * are performed.
 * @param[in] btree The btree to search in.
 * @param[in] key The key to search for.
 */
CSTRUCTURES_PUBLIC_API void*
btree_find(const struct btree_t* btree, btree_key_t key);

/*!
 * @brief Searches for a key that matches the specified value.
 * @note Complexity is O(n).
 * @param[in] btree The btree to search.
 * @param[in] value The value to search for.
 * @return Returns the key if it was successfully found, or BTREE_INVALID_KEY if
 * otherwise.
 */
CSTRUCTURES_PUBLIC_API btree_key_t*
btree_find_key(const struct btree_t* btree, const void* value);

/*!
 * @brief Searches for the value associated with the specified key and compares
 * the memory of that value with the memory pointed to by "value".
 * @param[in] btree The tree to search.
 * @param[in] key The key to search for.
 * @param[in] value A pointer to the value to compare. The pointed-to memory
 * block must be at least the size of btree_value_size().
 * @return Returns a logical "true" if the key was found and the values match.
 * Returns a logical "false" otherwise (i.e. 0)
 */
CSTRUCTURES_PUBLIC_API int
btree_find_and_compare(const struct btree_t* btree, btree_key_t key, const void* value);

/*!
 * @brief Gets any item from the btree.
 *
 * This is useful when you want to iterate and remove all items from the btree
 * at the same time.
 * @return Returns a pointer to a value in the tree. Which item is implementation
 * specific, but deterministic.
 */
CSTRUCTURES_PUBLIC_API void*
btree_get_any_value(const struct btree_t* btree);

/*!
 * @brief Returns 1 if the specified key exists, 0 if otherwise.
 * @param btree The btree to find the key in.
 * @param key The key to search for.
 * @return Returns a logical "true" if the key was found. Otherwise, returns
 * a logical "false" (i.e. 0).
 */
CSTRUCTURES_PUBLIC_API int
btree_key_exists(struct btree_t* btree, btree_key_t key);

/*!
 * @brief Returns a key that does not yet exist in the btree.
 * @note Complexity can be up to O(n) in the worst case. O(1) in the best case.
 * @param[in] btree The btree to generate a key from.
 * @return Returns a key that does not yet exist in the btree.
 */
CSTRUCTURES_PUBLIC_API btree_key_t
btree_find_unused_key(struct btree_t* btree);

/*!
 * @brief Erases an item from the btree matching the specified key.
 * @note Complexity is O(log2(n))
 * @param[in] btree The btree to erase from.
 * @param[in] key The key to search for.
 * @return Returns BTREE_OK if the key was found and erased successfully.
 * Returns BTREE_NOT_FOUND if the key was not found.
 */
CSTRUCTURES_PUBLIC_API enum btree_status_e
btree_erase(struct btree_t* btree, btree_key_t key);

/*!
 * @brief Erases an item from the btree by value.
 * @note Complexity is O(n)
 * @param[in] btree The btree to erase from.
 * @param[in] value A pointer to a chunk of memory that is at least the size of
 * btree_value_size(btree) and contains the value to compare. The value size is
 * specified during btree creation.
 * @return Returns the key that was associated with the value, if found. Otherwise
 * returns BTREE_INVALID_KEY;
 */
CSTRUCTURES_PUBLIC_API btree_key_t
btree_erase_value(struct btree_t* btree, const void* value);

btree_key_t
btree_erase_index(struct btree_t* btree, btree_size_t idx);

/*!
 * @brief A variation of btree_erase_value() where the value parameter points
 * into the btree structure. Such a pointer can be obtained with e.g. btree_find().
 * This version is much faster because the value isn't searched for.
 * @note Complexity is O(1)
 * @param[in] btree The btree to erase from.
 * @param[in] value A pointer to a value stored inside the btree's internal
 * memory.
 * @return Returns the key that was associated with the value.
 */
CSTRUCTURES_PUBLIC_API btree_key_t
btree_erase_internal_value(struct btree_t* btree, const void* value);

/*!
 * @brief Erases all items in the tree, but keeps the underlying memory.
 * @note This does **not** free existing items if they aren't in-place. If
 * you have inserted pointers to objects into your btree and call this, those
 * items will be lost and a memory leak will have been created.
 * @param[in] btree The btree to clear.
 */
CSTRUCTURES_PUBLIC_API void
btree_clear(struct btree_t* btree);

/*!
 * @brief Shrinks the underlying memory, usually via realloc(). If the btree
 * is empty, then the underlying memory will be freed.
 * @param[in] btree The tree to compact.
 */
CSTRUCTURES_PUBLIC_API void
btree_compact(struct btree_t* btree);

/*!
 * @brief Returns the number of items in the specified btree.
 * @param[in] btree The btree to count the items of.
 */
#define btree_count(btree) ((btree)->count)

/*!
 * @brief Returns the size of the value type being stored. The value size is
 * specified during btree creation.
 * @return The size of the value type being stored in bytes.
 */
#define btree_value_size(btree) ((btree)->value_size)

/*!
 * @brief Returns the current capacity of the btree. This can be used to determine
 * when to call btree_compact(), for example.
 * @return Returns the number of items that would fit into the underlying buffer.
 * This value is always greater or equal to btree_count().
 */
#define btree_capacity(btree)  ((btree)->capacity)

/*!
 * @brief Iterates over the specified btree's items and opens a FOR_EACH
 * scope.
 * @param[in] btree The btree to iterate.
 * @param[in] T The type of data being held in the btree.
 * @param[in] k The name to give the variable holding the current key. Will
 * be of type btree_key_t.
 * @param[in] v The name to give the variable pointing to the current
 * item. Will be of type T*.
 */
#define BTREE_FOR_EACH(btree, T, k, v) {                                      \
    assert(btree_value_size(btree) > 0);                                      \
    btree_size_t idx_##k;                                                     \
    btree_key_t k;                                                            \
    T* v;                                                                     \
    for(idx_##k = 0;                                                          \
        idx_##k != btree_count(btree) && (                                    \
            ((k = *BTREE_KEY(btree, idx_##k)) || 1) &&                        \
            (((v  = (T*)BTREE_VALUE(btree, idx_##k)) != NULL) || 1));         \
        ++idx_##k) {

/*!
 * @brief Iterates over the specified btree's keys and opens a FOR_EACH scope.
 */
#define BTREE_KEYS_FOR_EACH(btree, k) {                                       \
    btree_size_t idx_##k;                                                     \
    btree_key_t k;                                                            \
    for(idx_##k = 0;                                                          \
        idx_##k != btree_count(btree) && ((k = *BTREE_KEY(btree, idx_##k)) || 1); \
        idx_##k++) {

/*!
 * @brief Closes a for each scope previously opened by BTREE_FOR_EACH.
 */
#define BTREE_END_EACH }}

/*!
 * @brief Will erase the current item in a for loop from the btree. The current
 * key and value variables become invalid. However, you can use the "continue"
 * keyword to obtain the next valid key/value pair.
 * @param[in] btree A pointer to the btree object currently being iterated.
 * @param[in] k The name of the active "key" variable. This should be identical
 * to the parameter passed to BTREE_FOR_EACH or BTREE_KEYS_FOR_EACH.
 * as in BTREE_FOR_EACH.
 */
#define BTREE_ERASE_CURRENT_ITEM_IN_FOR_LOOP(btree, k) do {                   \
        btree_erase_index(btree, idx_##k);                                    \
        idx_##k--;                                                            \
    } while(0)

C_END

#endif /* CSTRUCTURES_BTREE_H */
