#include "gmock/gmock.h"
#include "cstructures/vector.h"

#define NAME vector

using namespace ::testing;

TEST(NAME, init)
{
    struct vector_t vec;

    vec.capacity = 45;
    vec.count = 384;
    vec.data = (uint8_t*)4859;
    vec.element_size = 183;
    vector_init(&vec, sizeof(int));

    EXPECT_THAT(vec.capacity, Eq(0u));
    EXPECT_THAT(vec.count, Eq(0u));
    EXPECT_THAT(vec.data, IsNull());
    ASSERT_EQ(sizeof(int), vec.element_size);
}

TEST(NAME, create_initialises_vector)
{
    struct vector_t* vec;
    vector_create(&vec, sizeof(int));
    EXPECT_THAT(vec->capacity, Eq(0u));
    EXPECT_THAT(vector_count(vec), Eq(0u));
    EXPECT_THAT(vec->data, IsNull());
    ASSERT_EQ(sizeof(int), vec->element_size);
    vector_free(vec);
}

TEST(NAME, push_increments_count_and_causes_realloc_by_factor)
{
    struct vector_t* vec;
    vector_create(&vec, sizeof(int));
    int x = 9;

    for(int i = 0; i != CSTRUCTURES_VEC_MIN_CAPACITY; ++i)
        vector_push(vec, &x);

    EXPECT_THAT(vector_count(vec), Eq(CSTRUCTURES_VEC_MIN_CAPACITY));
    EXPECT_THAT(vec->capacity, Eq(CSTRUCTURES_VEC_MIN_CAPACITY));

    vector_push(vec, &x);
    EXPECT_THAT(vector_count(vec), Eq(CSTRUCTURES_VEC_MIN_CAPACITY + 1));
    EXPECT_THAT(vec->capacity, Eq(CSTRUCTURES_VEC_MIN_CAPACITY * CSTRUCTURES_VEC_EXPAND_FACTOR));

    vector_free(vec);
}

TEST(NAME, clear_keeps_buffer_and_resets_count)
{
    struct vector_t* vec;
    vector_create(&vec, sizeof(int));
    int x = 9;
    for(int i = 0; i != CSTRUCTURES_VEC_MIN_CAPACITY*2; ++i)
        vector_push(vec, &x);

    EXPECT_THAT(vector_count(vec), Eq(CSTRUCTURES_VEC_MIN_CAPACITY*2));
    EXPECT_THAT(vec->capacity, Eq(CSTRUCTURES_VEC_MIN_CAPACITY*2));
    vector_clear(vec);
    EXPECT_THAT(vector_count(vec), Eq(0u));
    EXPECT_THAT(vec->capacity, Eq(CSTRUCTURES_VEC_MIN_CAPACITY*2));
    EXPECT_THAT(vec->data, NotNull());

    vector_free(vec);
}

TEST(NAME, clear_and_compact_deletes_buffer_and_resets_count)
{
    struct vector_t* vec;
    vector_create(&vec, sizeof(int));
    int x = 9;
    vector_push(vec, &x);
    vector_clear(vec);
    vector_compact(vec);
    EXPECT_THAT(vector_count(vec), Eq(0u));
    EXPECT_THAT(vec->capacity, Eq(0u));
    EXPECT_THAT(vec->data, IsNull());
    vector_free(vec);
}

TEST(NAME, push_emplace_increments_count_and_causes_realloc_by_factor)
{
    struct vector_t* vec;
    void* emplaced;
    vector_create(&vec, sizeof(int));

    for(int i = 0; i != CSTRUCTURES_VEC_MIN_CAPACITY; ++i)
        vector_emplace(vec, &emplaced);

    EXPECT_THAT(vector_count(vec), Eq(CSTRUCTURES_VEC_MIN_CAPACITY));
    EXPECT_THAT(vec->capacity, Eq(CSTRUCTURES_VEC_MIN_CAPACITY));

    vector_emplace(vec, &emplaced);
    EXPECT_THAT(vector_count(vec), Eq(CSTRUCTURES_VEC_MIN_CAPACITY + 1));
    EXPECT_THAT(vec->capacity, Eq(CSTRUCTURES_VEC_MIN_CAPACITY * CSTRUCTURES_VEC_EXPAND_FACTOR));

    vector_free(vec);
}

TEST(NAME, pop_returns_pushed_values)
{
    struct vector_t* vec;
    vector_create(&vec, sizeof(int));
    int x;

    x = 3; vector_push(vec, &x);
    x = 2; vector_push(vec, &x);
    x = 6; vector_push(vec, &x);
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(6));
    x = 23; vector_push(vec, &x);
    x = 21; vector_push(vec, &x);
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(21));
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(23));
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(2));
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(3));

    EXPECT_THAT(vector_count(vec), Eq(0u));
    EXPECT_THAT(vec->data, NotNull());

    vector_free(vec);
}

TEST(NAME, pop_returns_push_emplaced_values)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));

    vector_emplace(vec, (void**)&emplaced); *emplaced = 53;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 24;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 73;
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(73));
    vector_emplace(vec, (void**)&emplaced); *emplaced = 28;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 72;
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(72));
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(28));
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(24));
    EXPECT_THAT(*(int*)vector_pop(vec), Eq(53));

    EXPECT_THAT(vector_count(vec), Eq(0u));
    EXPECT_THAT(vec->data, NotNull());

    vector_free(vec);
}

TEST(NAME, pop_empty_vector)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    vector_emplace(vec, (void**)&emplaced); *emplaced = 21;
    vector_pop(vec);
    EXPECT_THAT(vector_pop(vec), IsNull());
    EXPECT_THAT(vector_count(vec), Eq(0u));
    EXPECT_THAT(vec->data, NotNull());
    vector_free(vec);
}

TEST(NAME, pop_clear_freed_vector)
{
    struct vector_t* vec; vector_create(&vec, sizeof(int));
    EXPECT_THAT(vector_pop(vec), IsNull());
    EXPECT_THAT(vector_count(vec), Eq(0u));
    EXPECT_THAT(vec->capacity, Eq(0u));
    EXPECT_THAT(vec->data, IsNull());
    vector_free(vec);
}

TEST(NAME, get_element_random_access)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    vector_emplace(vec, (void**)&emplaced); *emplaced = 53;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 24;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 73;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 43;
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(53));
    vector_free(vec);
}

TEST(NAME, popping_preserves_existing_elements)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    vector_emplace(vec, (void**)&emplaced); *emplaced = 53;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 24;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 73;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 43;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 24;
    vector_pop(vec);
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(53));
    vector_free(vec);
}

TEST(NAME, erasing_by_index_preserves_existing_elements)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    vector_emplace(vec, (void**)&emplaced); *emplaced = 53;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 24;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 73;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 43;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 65;
    vector_erase_index(vec, 1);
    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(65));
    vector_erase_index(vec, 1);
    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(65));
    vector_free(vec);
}

TEST(NAME, erasing_by_element_preserves_existing_elements)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    vector_emplace(vec, (void**)&emplaced); *emplaced = 53;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 24;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 73;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 43;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 65;
    vector_erase_element(vec, vector_get_element(vec, 1));
    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(65));
    vector_erase_element(vec, vector_get_element(vec, 1));
    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(65));
    vector_free(vec);
}

TEST(NAME, get_invalid_index)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    EXPECT_THAT(vector_get_element(vec, 1), IsNull());
    vector_emplace(vec, (void**)&emplaced); *emplaced = 53;
    EXPECT_THAT(vector_get_element(vec, 1), IsNull());
    vector_free(vec);
}

TEST(NAME, erase_invalid_index)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    vector_erase_index(vec, 1);
    vector_erase_index(vec, 0);
    vector_emplace(vec, (void**)&emplaced); *emplaced = 53;
    vector_erase_index(vec, 1);
    vector_erase_index(vec, 0);
    vector_erase_index(vec, 0);
    vector_free(vec);
}

TEST(NAME, inserting_preserves_existing_elements)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    vector_emplace(vec, (void**)&emplaced); *emplaced = 53;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 24;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 73;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 43;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 65;

    int x = 68;
    vector_insert(vec, 2, &x); // middle insertion

    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(68));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 4), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 5), Eq(65));

    x = 16;
    vector_insert(vec, 0, &x); // beginning insertion

    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(16));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(68));
    EXPECT_THAT(*(int*)vector_get_element(vec, 4), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 5), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 6), Eq(65));

    x = 82;
    vector_insert(vec, 7, &x); // end insertion

    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(16));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(68));
    EXPECT_THAT(*(int*)vector_get_element(vec, 4), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 5), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 6), Eq(65));
    EXPECT_THAT(*(int*)vector_get_element(vec, 7), Eq(82));

    x = 37;
    vector_insert(vec, 7, &x); // end insertion

    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(16));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(68));
    EXPECT_THAT(*(int*)vector_get_element(vec, 4), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 5), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 6), Eq(65));
    EXPECT_THAT(*(int*)vector_get_element(vec, 7), Eq(37));
    EXPECT_THAT(*(int*)vector_get_element(vec, 8), Eq(82));

    vector_free(vec);
}

TEST(NAME, insert_emplacing_preserves_existing_elements)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    vector_emplace(vec, (void**)&emplaced); *emplaced = 53;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 24;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 73;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 43;
    vector_emplace(vec, (void**)&emplaced); *emplaced = 65;

    vector_insert_emplace(vec, 2, (void**)&emplaced); *emplaced = 68; // middle insertion

    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(68));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 4), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 5), Eq(65));

    vector_insert_emplace(vec, 0, (void**)&emplaced); *emplaced = 16; // beginning insertion

    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(16));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(68));
    EXPECT_THAT(*(int*)vector_get_element(vec, 4), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 5), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 6), Eq(65));

    vector_insert_emplace(vec, 7, (void**)&emplaced); *emplaced = 82; // end insertion

    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(16));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(68));
    EXPECT_THAT(*(int*)vector_get_element(vec, 4), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 5), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 6), Eq(65));
    EXPECT_THAT(*(int*)vector_get_element(vec, 7), Eq(82));

    vector_insert_emplace(vec, 7, (void**)&emplaced); *emplaced = 37; // end insertion

    EXPECT_THAT(*(int*)vector_get_element(vec, 0), Eq(16));
    EXPECT_THAT(*(int*)vector_get_element(vec, 1), Eq(53));
    EXPECT_THAT(*(int*)vector_get_element(vec, 2), Eq(24));
    EXPECT_THAT(*(int*)vector_get_element(vec, 3), Eq(68));
    EXPECT_THAT(*(int*)vector_get_element(vec, 4), Eq(73));
    EXPECT_THAT(*(int*)vector_get_element(vec, 5), Eq(43));
    EXPECT_THAT(*(int*)vector_get_element(vec, 6), Eq(65));
    EXPECT_THAT(*(int*)vector_get_element(vec, 7), Eq(37));
    EXPECT_THAT(*(int*)vector_get_element(vec, 8), Eq(82));

    vector_free(vec);
}

TEST(NAME, resizing_larger_than_capacity_reallocates_and_updates_size)
{
    struct vector_t* vec;
    int* old_ptr;
    vector_create(&vec, sizeof(int));

    vector_emplace(vec, (void**)&old_ptr);
    *old_ptr = 42;
    vector_resize(vec, 64);
    int* new_ptr = (int*)vector_get_element(vec, 0);
    EXPECT_THAT(old_ptr, Ne(new_ptr));
    EXPECT_THAT(*new_ptr, Eq(42));
    EXPECT_THAT(vec->capacity, Eq(64u));
    EXPECT_THAT(vector_count(vec), Eq(64u));

    vector_free(vec);
}

TEST(NAME, resizing_smaller_than_capacity_updates_size_but_not_capacity)
{
    struct vector_t* vec;
    int* emplaced;
    vector_create(&vec, sizeof(int));
    vector_emplace(vec, (void**)&emplaced);
    vector_resize(vec, 64);

    EXPECT_THAT(vec->capacity, Eq(64u));
    EXPECT_THAT(vector_count(vec), Eq(64u));

    vector_resize(vec, 8);

    EXPECT_THAT(vec->capacity, Eq(64u));
    EXPECT_THAT(vector_count(vec), Eq(8u));

    vector_free(vec);
}
