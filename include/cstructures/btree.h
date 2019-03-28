/*!
 * @file btree.h
 * @brief Implements a container of ordered key-value pairs stored in a vector
 * (ordered by key). The key is computed from a key (string) provided by the
 * user.
 */

#ifndef BTREE_H
#define BTREE_H

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

enum btree_status_e
{
    BTREE_NOT_FOUND    = 2,
    BTREE_EXISTS       = 1,
    BTREE_OK           = 0,
    BTREE_OOM          = -1
};

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
 * @note This does **not** FREE existing elements. If you have elements in your
 * btree and call this, those elements will be lost and a memory leak will have
 * been created.
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
 * @brief Inserts an element into the btree using a key.
 *
 * @note Complexity is O(log2(n)) to find the insertion point.
 *
 * @param[in] btree The btree object to insert into.
 * @param[in] key A unique key to assign to the element being inserted. The
 * key must not exist in the btree, or the element will not be inserted.
 * @param[in] value A pointer to the data to insert into the tree. The data
 * pointed to is copied into the structure. If you are storing pointers to
 * strings (having specified sizeof(void*) for value_size), you would pass
 * a double-pointer to the string for the pointer to be copied into the btree.
 * @return Returns 0 if insertion was successful. Returns 1 if the key already
 * existed (in which case nothing is inserted). Returns -1 on failure.
 */
CSTRUCTURES_PUBLIC_API enum btree_status_e
btree_insert(struct btree_t* btree, btree_key_t key, void* value);

/*!
 * @brief Updates an existing value. If the key doesn't exist, this function
 * does nothing.
 * @note If the key is not found, this function silently fails.
 * @param[in] btree A pointer to the btree object to change the value of.
 * @param[in] key The unique key associated with the value you want to change.
 * @param[in] value The new value to set.
 */
CSTRUCTURES_PUBLIC_API void
btree_set_existing(struct btree_t* btree, btree_key_t key, void* value);

CSTRUCTURES_PUBLIC_API enum btree_status_e
btree_set_or_insert(struct btree_t* btree, btree_key_t key, void* value);

/*!
 * @brief Looks for an element in the btree and returns a pointer to the element
 * in the structure. This is useful if you need to store data directly in the
 * memory occupied by the pointer and wish to modify it.
 * @note The returned pointer can be invalidated if any insertions or deletions
 * are performed.
 * @param[in] btree The btree to search in.
 * @param[in] key The key to search for.
 */
CSTRUCTURES_PUBLIC_API void*
btree_find(const struct btree_t* btree, btree_key_t key);

/*!
 * @brief Finds the specified element in the btree and returns a pointer to its
 * key.
 * @note Complexity is O(n).
 * @param[in] btree The btree to search.
 * @param[in] value The value to search for.
 * @return Returns the key if it was successfully found, or BTREE_INVALID_KEY if
 * otherwise.
 */
CSTRUCTURES_PUBLIC_API btree_key_t*
btree_find_key(const struct btree_t* btree, const void* value);

/*!
 * @brief Gets any element from the btree.
 *
 * This is useful when you want to iterate and remove all items from the btree
 * at the same time.
 * @return Returns an element as a void pointer. Which element is implementation
 * specific, but deterministic.
 */
CSTRUCTURES_PUBLIC_API void*
btree_get_any_value(const struct btree_t* btree);

/*!
 * @brief Returns 1 if the specified key exists, 0 if otherwise.
 * @param btree The btree to find the key in.
 * @param key The key to search for.
 * @return 0 if the key was found, -1 if the key was not found.
 */
CSTRUCTURES_PUBLIC_API int
btree_key_exists(struct btree_t* btree, btree_key_t key);

/*!
 * @brief Returns a key that does not yet exist in the btree.
 * @note Complexity is O(n)
 * @param[in] btree The btree to generate a key from.
 * @return Returns a key that does not yet exist in the btree.
 */
CSTRUCTURES_PUBLIC_API btree_key_t
btree_find_unused_key(struct btree_t* btree);

/*!
 * @brief Erases an element from the btree using a key.
 * @warning It is highly discouraged to mix btree_erase_using_key() and
 * btree_erase_using_key(). Use btree_erase_using_key() if you used
 * btree_insert_using_key(). Use btree_erase_using_key() if you used
 * btree_insert_using_key().
 * @note Complexity is O(log2(n))
 * @param[in] btree The btree to erase from.
 * @param[in] key The key that btrees to the element to remove from the btree.
 * @return Returns the data assocated with the specified key. If the key is
 * not found in the btree, NULL is returned.
 * @note The btree only holds references to values and does **not** FREE them. It
 * is up to the programmer to correctly free the elements being erased from the
 * btree.
 */
CSTRUCTURES_PUBLIC_API void
btree_erase(struct btree_t* btree, btree_key_t key);

CSTRUCTURES_PUBLIC_API btree_key_t
btree_erase_value(struct btree_t* btree, void* value);

/*!
 * @brief Erases the entire btree, including the underlying memory.
 * @note This does **not** FREE existing elements. If you have elements in your
 * btree and call this, those elements will be lost and a memory leak will have
 * been created.
 * @param[in] btree The btree to clear.
 */
CSTRUCTURES_PUBLIC_API void
btree_clear(struct btree_t* btree);

CSTRUCTURES_PUBLIC_API void
btree_compact(struct btree_t* btree);

/*!
 * @brief Returns the number of elements in the specified btree.
 * @param[in] btree The btree to count the elements of.
 * @return The number of elements in the specified btree.
 */
#define btree_count(btree) ((btree)->count)

/*!
 * @brief Iterates over the specified btree's elements and opens a FOR_EACH
 * scope.
 * @param[in] btree The btree to iterate.
 * @param[in] T The type of data being held in the btree.
 * @param[in] k The name to give the variable holding the current key. Will
 * be of type btree_key_t.
 * @param[in] v The name to give the variable pointing to the current
 * element. Will be of type T*.
 */
#define BTREE_FOR_EACH(btree, T, k, v) {                                                           \
    btree_key_t btree_##v;                                                                         \
    btree_key_t k;                                                                                 \
    T* v;                                                                                          \
    for(btree_##v = 0;                                                                             \
        btree_##v != btree_count(btree) &&                                                         \
            ((k = ((struct btree_key_value_t*) (btree)->vector.data)[btree_##v].key) || 1) &&      \
            ((v  = (T*)((struct btree_key_value_t*)(btree)->vector.data)[btree_##v].value) || 1);  \
        ++btree_##v)

/*!
 * @brief Closes a for each scope previously opened by BTREE_FOR_EACH.
 */
#define BTREE_END_EACH }

/*!
 * @brief Will erase the current selected item in a for loop from the btree.
 * @note This does not free the data being referenced by the btree. You will have
 * to erase that manually (either before or after this operation, it doesn't
 * matter).
 * @param[in] btree A pointer to the btree object currently being iterated.
 */
#define BTREE_ERASE_CURRENT_ITEM_IN_FOR_LOOP(btree, v) do {                                          \
    vector_erase_element(&(btree)->vector, ((btree_key_value_t*)(btree)->vector.data) + btree_##v); \
    --btree_##v; } while(0)

C_END

#endif /* BTREE_H */
