/*!
 * @file vector.h
 * @brief Dynamic contiguous sequence container with guaranteed element order.
 * @page vector Ordered Vector
 *
 * Ordered vectors arrange all inserted elements next to each other in memory.
 * Because of this, vector access is just as efficient as a normal array, but
 * they are able to grow and shrink in size automatically.
 */

#ifndef CSTRUCTURES_VECTOR_H
#define CSTRUCTURES_VECTOR_H

#include "cstructures/config.h"
#include <stdint.h>

C_BEGIN

#if defined(CSTRUCTURES_VEC_64BIT)
typedef uint64_t vec_size_t;
#else
typedef uint32_t vec_size_t;
#endif
typedef intptr_t vec_idx_t;

enum vec_status_e
{
    VEC_OK  = 0,
    VEC_OOM = -1,
    VEC_DIFFERENT_ELEMENT_SIZES = -2
};

struct vec_t
{
    uint8_t* data;            /* pointer to the contiguous section of memory */
    vec_size_t capacity;      /* how many elements actually fit into the allocated space */
    vec_size_t count;         /* number of elements inserted */
    vec_size_t element_size;  /* how large one element is in bytes */
};

/*!
 * @brief Creates a new vector object. See @ref vector for details.
 * @param[in] element_size Specifies the size in bytes of the type of data you want
 * the vector to store. Typically one would pass sizeof(my_data_type).
 * @return Returns the newly created vector object.
 */
CSTRUCTURES_PUBLIC_API enum vec_status_e
vector_create(struct vec_t** vec, const vec_size_t element_size);

/*!
 * @brief Initializes an existing vector object.
 * @note This does **not** free existing memory. If you've pushed elements
 * into your vector and call this, you will have created a memory leak.
 * @param[in] vector The vector to initialize.
 * @param[in] element_size Specifies the size in bytes of the type of data you
 * want the vector to store. Typically one would pass sizeof(my_data_type).
 */
CSTRUCTURES_PUBLIC_API enum vec_status_e
vector_init(struct vec_t* vector,
            const vec_size_t element_size);

CSTRUCTURES_PUBLIC_API void
vector_deinit(struct vec_t* vector);

/*!
 * @brief Destroys an existing vector object and frees all memory allocated by
 * inserted elements.
 * @param[in] vector The vector to free.
 */
CSTRUCTURES_PUBLIC_API void
vector_free(struct vec_t* vector);

/*!
 * @brief Erases all elements in a vector.
 * @note This does not actually erase the underlying memory, it simply resets
 * the element counter. If you wish to free the underlying memory, see
 * vector_clear_free().
 * @param[in] vector The vector to clear.
 */
CSTRUCTURES_PUBLIC_API void
vector_clear(struct vec_t* vector);

/*!
 * @brief Erases all elements in a vector and frees their memory.
 * @param[in] vector The vector to clear.
 */
CSTRUCTURES_PUBLIC_API void
vector_compact(struct vec_t* vector);

CSTRUCTURES_PUBLIC_API enum vec_status_e
vector_reserve(struct vec_t* vector, vec_size_t size);

/*!
 * @brief Sets the size of the vector to exactly the size specified. If the
 * vector was smaller then memory will be reallocated. If the vector was larger
 * then the capacity will remain the same and the size will adjusted.
 * @param[in] vector The vector to resize.
 * @param[in] size The new size of the vector.
 * @return Returns VEC_OOM on failure, CSTRUCTURES_OK on success.
 */
CSTRUCTURES_PUBLIC_API enum vec_status_e
vector_resize(struct vec_t* vector, vec_size_t size);

/*!
 * @brief Gets the number of elements that have been inserted into the vector.
 */
#define vector_count(x) ((x)->count)

#define vector_capacity(x) ((x)->capacity)

#define vector_data(x) ((x)->data)

/*!
 * @brief Inserts (copies) a new element at the head of the vector.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time a
 * re-allocation occurs to cut down on the frequency of re-allocations.
 * @note If you do not wish to copy data into the vector, but merely make
 * space, see vector_push_emplace().
 * @param[in] vector The vector to push into.
 * @param[in] data The data to copy into the vector. It is assumed that
 * sizeof(data) is equal to what was specified when the vector was first
 * created. If this is not the case then it could cause undefined behaviour.
 * @return Returns CSTRUCTURES_OK if the data was successfully pushed, VEC_OOM
 * if otherwise.
 */
CSTRUCTURES_PUBLIC_API enum vec_status_e
vector_push(struct vec_t* vector, const void* data);

/*!
 * @brief Allocates space for a new element at the head of the vector, but does
 * not initialize it.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] vector The vector to emplace an element into.
 * @return A pointer to the allocated memory for the requested element. See
 * warning and use with caution.
 */
CSTRUCTURES_PUBLIC_API enum vec_status_e
vector_emplace(struct vec_t* vector, void** data);

/*!
 * @brief Copies the contents of another vector and pushes it into the vector.
 * @return Returns CSTRUCTURES_OK if successful, VEC_OOM if otherwise.
 */
CSTRUCTURES_PUBLIC_API enum vec_status_e
vector_push_vector(struct vec_t* vector, const struct vec_t* source_vector);

/*!
 * @brief Removes an element from the back (end) of the vector.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] vector The vector to pop an element from.
 * @return A pointer to the popped element. See warning and use with caution.
 * If there are no elements to pop, NULL is returned.
 */
CSTRUCTURES_PUBLIC_API void*
vector_pop(struct vec_t* vector);

/*!
 * @brief Returns the very last element of the vector.
 * @warning The returned pointer could be invalidated if any other vector
 * related function is called, as the underlying memory of the vector could be
 * re-allocated. Use the pointer immediately after calling this function.
 *
 * @param[in] vector The vector to return the last element from.
 * @return A pointer to the last element. See warning and use with caution.
 * If there are no elements in the vector, NULL is returned.
 */
CSTRUCTURES_PUBLIC_API void*
vector_back(const struct vec_t* vector);

/*!
 * @brief Allocates space for a new element at the specified index, but does
 * not initialize it.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time a
 * re-allocation occurs to cut down on the frequency of re-allocations.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] vector The vector to emplace an element into.
 * @param[in] index Where to insert.
 * @return A pointer to the emplaced element. See warning and use with caution.
 */
CSTRUCTURES_PUBLIC_API enum vec_status_e
vector_insert_emplace(struct vec_t* vector, vec_idx_t index, void** data);

/*!
 * @brief Inserts (copies) a new element at the specified index.
 * @note This can cause a re-allocation of the underlying memory. This
 * implementation expands the allocated memory by a factor of 2 every time a
 * re-allocation occurs to cut down on the frequency of re-allocations.
 * @note If you do not wish to copy data into the vector, but merely make
 * space, see vector_insert_emplace().
 * @param[in] vector The vector to insert into.
 * @param[in] data The data to copy into the vector. It is assumed that
 * sizeof(data) is equal to what was specified when the vector was first
 * created. If this is not the case then it could cause undefined behaviour.
 */
CSTRUCTURES_PUBLIC_API enum vec_status_e
vector_insert(struct vec_t* vector, vec_idx_t index, void* data);

/*!
 * @brief Erases the specified element from the vector.
 * @note This causes all elements with indices greater than **index** to be
 * re-allocated (shifted 1 element down) so the vector remains contiguous.
 * @param[in] index The position of the element in the vector to erase. The index
 * ranges from **0** to **vector_count()-1**.
 */
CSTRUCTURES_PUBLIC_API void
vector_erase_index(struct vec_t* vector, vec_idx_t index);

/*!
 * @brief Removes the element in the vector pointed to by **element**.
 * @param[in] vector The vector from which to erase the data.
 * @param[in] element A pointer to an element within the vector.
 * @note The pointer must point into the vector's data.
 */
CSTRUCTURES_PUBLIC_API void
vector_erase_element(struct vec_t* vector, void* element);

/*!
 * @brief Gets a pointer to the specified element in the vector.
 * @warning The returned pointer could be invalidated if any other
 * vector related function is called, as the underlying memory of the vector
 * could be re-allocated. Use the pointer immediately after calling this
 * function.
 * @param[in] vector The vector to get the element from.
 * @param[in] index The index of the element to get. The index ranges from
 * **0** to **vector_count()-1**.
 * @return [in] A pointer to the element. See warning and use with caution.
 * If the specified element doesn't exist (index out of bounds), NULL is
 * returned.
 */
CSTRUCTURES_PUBLIC_API void*
vector_get_element(const struct vec_t*, vec_idx_t index);

CSTRUCTURES_PUBLIC_API vec_idx_t
vector_find_element(const struct vec_t* vector, void* element);

/*!
 * @brief Convenient macro for iterating a vector's elements.
 *
 * Example:
 * ```
 * vec_t* some_vector = (a vector containing elements of type "bar")
 * VECTOR_FOR_EACH(some_vector, bar, element)
 * {
 *     do_something_with(element);  ("element" is now of type "bar*")
 * }
 * ```
 * @param[in] vector A pointer to the vector to iterate.
 * @param[in] var_type Should be the type of data stored in the vector.
 * @param[in] var The name of a temporary variable you'd lcstructurese to use within the
 * for-loop to reference the current element.
 */
#define VECTOR_FOR_EACH(vector, var_type, var) {                             \
    var_type* var;                                                           \
    uint8_t* internal_##var_end_of_vector = (vector)->data + (vector)->count * (vector)->element_size; \
    for(var = (var_type*)(vector)->data;                                     \
        (uint8_t*)var != internal_##var_end_of_vector;                       \
        var = (var_type*)(((uint8_t*)var) + (vector)->element_size))


#define VECTOR_FOR_EACH_R(vector, var_type, var) {                           \
    var_type* var;                                                           \
    uint8_t* internal_##var_start_of_vector = (vector)->data - (vector)->element_size; \
    for(var = (var_type*)((vector)->data + (vector)->count * (vector)->element_size - (vector)->element_size); \
        (uint8_t*)var != internal_##var_start_of_vector;                     \
        var = (var_type*)(((uint8_t*)var) - (vector)->element_size))

/*!
 * @brief Convenient macro for iterating a range of a vector's elements.
 * @param[in] vector A pointer to the vector to iterate.
 * @param[in] var_type Should be the type of data stored in the vector. For
 * example, if your vector is storing ```type_t*``` objects then
 * var_type should equal ```type_t``` (without the pointer).
 * @param[in] var The name of a temporary variable you'd lcstructurese to use within the
 * for loop to reference the current element.
 * @param[in] begin_index The index (starting at 0) of the first element to
 * start with.
 * @param[in] end_index The index of the last element to iterate (exclusive).
 */
#define VECTOR_FOR_EACH_RANGE(vector, var_type, var, begin_index, end_index) { \
    var_type* var;                                                             \
    uint8_t* internal_##var_end_of_vector = (vector)->data + end_index * (vector)->element_size; \
    for(var = (var_type*)((vector)->data + begin_index * (vector)->element_size); \
        (uint8_t*)var != internal_##var_end_of_vector;                         \
        var = (var_type*)(((uint8_t*)var) + (vector)->element_size))

/*!
 * @brief Closes a for each scope previously opened by VECTOR_FOR_EACH.
 */
#define VECTOR_END_EACH }

/*!
 * @brief Convenient macro for erasing an element while iterating a vector.
 * @warning Only call this while iterating.
 * Example:
 * ```
 * VECTOR_FOR_EACH(some_vector, bar, element)
 * {
 *     VECTOR_ERASE_IN_FOR_LOOP(some_vector, bar, element);
 * }
 * ```
 * @param[in] vector The vector to erase from.
 * @param[in] var_type Should be the type of data stored in the vector.
 * @param[in] element The element to erase.
 */
#define VECTOR_ERASE_IN_FOR_LOOP(vector, element_type, element)              \
    vector_erase_element(vector, element);                                   \
    element = (element_type*)(((uint8_t*)element) - (vector)->element_size); \
    internal_##var_end_of_vector = (vector)->data + (vector)->count * (vector)->element_size;

C_END

#endif /* CSTRUCTURES_VECTOR_H */
