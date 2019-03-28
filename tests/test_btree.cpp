#include "gmock/gmock.h"
#include "cstructures/btree.h"

#define NAME btree

using namespace testing;

struct data_t
{
    float x, y, z;
};

TEST(NAME, init_sets_correct_values)
{
    struct btree_t btree;
    btree.count = 4;
    btree.capacity = 56;
    btree.data = (uint8_t*)4783;
    btree.value_size = 283;

    ASSERT_THAT(btree_init(&btree, sizeof(data_t)), Eq(BTREE_OK));
    EXPECT_THAT(btree.count, Eq(0u));
    EXPECT_THAT(btree.capacity, Eq(0u));
    EXPECT_THAT(btree.count, Eq(0u));
    EXPECT_THAT(btree.data, IsNull());
    EXPECT_THAT(btree.value_size, Eq(sizeof(data_t)));
}

TEST(NAME, create_initializes_btree)
{
    struct btree_t* btree;
    ASSERT_THAT(btree_create(&btree, sizeof(data_t)), Eq(BTREE_OK));
    EXPECT_THAT(btree->capacity, Eq(0u));
    EXPECT_THAT(btree->count, Eq(0u));
    EXPECT_THAT(btree->data, IsNull());
    EXPECT_THAT(btree->value_size, Eq(sizeof(data_t)));
    btree_free(btree);
}

TEST(NAME, insertion_forwards)
{
    struct btree_t* btree;
    ASSERT_THAT(btree_create(&btree, sizeof(int)), Eq(BTREE_OK));

    int a=56, b=45, c=18, d=27, e=84;
    ASSERT_THAT(btree_insert(btree, 0, &a), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(1u));
    ASSERT_THAT(btree_insert(btree, 1, &b), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(2u));
    ASSERT_THAT(btree_insert(btree, 2, &c), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(3u));
    ASSERT_THAT(btree_insert(btree, 3, &d), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(4u));
    ASSERT_THAT(btree_insert(btree, 4, &e), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(5u));

    EXPECT_THAT(*(int*)btree_find(btree, 0), Eq(a));
    EXPECT_THAT(*(int*)btree_find(btree, 1), Eq(b));
    EXPECT_THAT(*(int*)btree_find(btree, 2), Eq(c));
    EXPECT_THAT(*(int*)btree_find(btree, 3), Eq(d));
    EXPECT_THAT(*(int*)btree_find(btree, 4), Eq(e));
    EXPECT_THAT(btree_find(btree, 5), IsNull());

    btree_free(btree);
}

TEST(NAME, insertion_backwards)
{
    struct btree_t* btree;
    btree_create(&btree, sizeof(int));

    int a=56, b=45, c=18, d=27, e=84;
    ASSERT_THAT(btree_insert(btree, 4, &a), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(1u));
    ASSERT_THAT(btree_insert(btree, 3, &b), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(2u));
    ASSERT_THAT(btree_insert(btree, 2, &c), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(3u));
    ASSERT_THAT(btree_insert(btree, 1, &d), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(4u));
    ASSERT_THAT(btree_insert(btree, 0, &e), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(5u));

    EXPECT_THAT(*(int*)btree_find(btree, 0), Eq(e));
    EXPECT_THAT(*(int*)btree_find(btree, 1), Eq(d));
    EXPECT_THAT(*(int*)btree_find(btree, 2), Eq(c));
    EXPECT_THAT(*(int*)btree_find(btree, 3), Eq(b));
    EXPECT_THAT(*(int*)btree_find(btree, 4), Eq(a));
    EXPECT_THAT(btree_find(btree, 5), IsNull());

    btree_free(btree);
}

TEST(NAME, insertion_random)
{
    struct btree_t* btree;
    ASSERT_THAT(btree_create(&btree, sizeof(int)), Eq(BTREE_OK));

    int a=56, b=45, c=18, d=27, e=84;
    ASSERT_THAT(btree_insert(btree, 26, &a), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(1u));
    ASSERT_THAT(btree_insert(btree, 44, &b), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(2u));
    ASSERT_THAT(btree_insert(btree, 82, &c), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(3u));
    ASSERT_THAT(btree_insert(btree, 41, &d), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(4u));
    ASSERT_THAT(btree_insert(btree, 70, &e), Eq(BTREE_OK));  ASSERT_THAT(btree_count(btree), Eq(5u));

    EXPECT_THAT(*(int*)btree_find(btree, 26), Eq(a));
    EXPECT_THAT(*(int*)btree_find(btree, 41), Eq(d));
    EXPECT_THAT(*(int*)btree_find(btree, 44), Eq(b));
    EXPECT_THAT(*(int*)btree_find(btree, 70), Eq(e));
    EXPECT_THAT(*(int*)btree_find(btree, 82), Eq(c));

    btree_free(btree);
}

TEST(NAME, clear_keeps_underlying_buffer)
{
    struct btree_t* btree;
    ASSERT_THAT(btree_create(&btree, sizeof(int)), Eq(BTREE_OK));

    int a = 53;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &a);
    btree_insert(btree, 2, &a);

    // this should delete all entries but keep the underlying buffer
    btree_clear(btree);

    ASSERT_THAT(btree_count(btree), Eq(0u));
    EXPECT_THAT(btree->data, NotNull());
    EXPECT_THAT(btree->capacity, Ne(0u));

    btree_free(btree);
}

TEST(NAME, compact_reduces_capacity_and_keeps_elements_in_tact)
{
    struct btree_t* btree;
    ASSERT_THAT(btree_create(&btree, sizeof(int)), Eq(BTREE_OK));

    int a = 53;
    for (int i = 0; i != CSTRUCTURES_BTREE_MIN_CAPACITY * 3; ++i)
        ASSERT_THAT(btree_insert(btree, i, &a), Eq(BTREE_OK));
    for (int i = 0; i != CSTRUCTURES_BTREE_MIN_CAPACITY; ++i)
        btree_erase(btree, i);

    btree_size_t old_capacity = btree->capacity;
    btree_compact(btree);
    EXPECT_THAT(btree->capacity, Lt(old_capacity));
    EXPECT_THAT(btree_count(btree), Eq(CSTRUCTURES_BTREE_MIN_CAPACITY * 2));
    EXPECT_THAT(btree->capacity, Eq(CSTRUCTURES_BTREE_MIN_CAPACITY * 2));
    EXPECT_THAT(btree->data, NotNull());

    btree_free(btree);
}

TEST(NAME, clear_and_compact_deletes_underlying_buffer)
{
    struct btree_t* btree;
    ASSERT_THAT(btree_create(&btree, sizeof(int)), Eq(BTREE_OK));

    int a=53;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &a);
    btree_insert(btree, 2, &a);

    // this should delete all entries + free the underlying buffer
    btree_clear(btree);
    btree_compact(btree);

    ASSERT_THAT(btree_count(btree), Eq(0u));
    ASSERT_THAT(btree->data, IsNull());
    ASSERT_THAT(btree->capacity, Eq(0u));

    btree_free(btree);
}
/*
TEST(NAME, count_returns_correct_number)
{
    struct btree_t* btree; btree_create(&btree);

    int a=53;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &a);
    btree_insert(btree, 2, &a);

    ASSERT_THAT(btree_count(btree), Eq(3u));

    btree_free(btree);
}

TEST(NAME, erase_elements)
{
    struct btree_t* btree; btree_create(&btree);

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &b);
    btree_insert(btree, 2, &c);
    btree_insert(btree, 3, &d);
    btree_insert(btree, 4, &e);

    ASSERT_THAT(*(int*)btree_erase(btree, 2), Eq(c));

    // 4
    ASSERT_THAT(*(int*)btree_find(btree, 0), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(d));
    ASSERT_THAT(*(int*)btree_find(btree, 4), Eq(e));

    ASSERT_THAT(*(int*)btree_erase(btree, 4), Eq(e));

    // 3
    ASSERT_THAT(*(int*)btree_find(btree, 0), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(d));

    ASSERT_THAT(*(int*)btree_erase(btree, 0), Eq(a));

    // 2
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(d));

    ASSERT_THAT(*(int*)btree_erase(btree, 1), Eq(b));

    // 1
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(d));

    ASSERT_THAT(*(int*)btree_erase(btree, 3), Eq(d));

    ASSERT_THAT(btree_erase(btree, 2), IsNull());

    btree_free(btree);
}

TEST(NAME, reinsertion_forwards)
{
    struct btree_t* btree; btree_create(&btree);

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &b);
    btree_insert(btree, 2, &c);
    btree_insert(btree, 3, &d);
    btree_insert(btree, 4, &e);

    btree_erase(btree, 4);
    btree_erase(btree, 3);
    btree_erase(btree, 2);

    btree_insert(btree, 2, &c);
    btree_insert(btree, 3, &d);
    btree_insert(btree, 4, &e);

    ASSERT_THAT( *(int*)btree_find(btree, 0), Eq(a));
    ASSERT_THAT( *(int*)btree_find(btree, 1), Eq(b));
    ASSERT_THAT( *(int*)btree_find(btree, 2), Eq(c));
    ASSERT_THAT( *(int*)btree_find(btree, 3), Eq(d));
    ASSERT_THAT( *(int*)btree_find(btree, 4), Eq(e));

    btree_free(btree);
}

TEST(NAME, reinsertion_backwards)
{
    struct btree_t* btree; btree_create(&btree);

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 4, &a);
    btree_insert(btree, 3, &b);
    btree_insert(btree, 2, &c);
    btree_insert(btree, 1, &d);
    btree_insert(btree, 0, &e);

    btree_erase(btree, 0);
    btree_erase(btree, 1);
    btree_erase(btree, 2);

    btree_insert(btree, 2, &c);
    btree_insert(btree, 1, &d);
    btree_insert(btree, 0, &e);

    ASSERT_THAT(*(int*)btree_find(btree, 0), Eq(e));
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(d));
    ASSERT_THAT(*(int*)btree_find(btree, 2), Eq(c));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 4), Eq(a));

    btree_free(btree);
}

TEST(NAME, reinsertion_random)
{
    struct btree_t* btree; btree_create(&btree);

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 26, &a);
    btree_insert(btree, 44, &b);
    btree_insert(btree, 82, &c);
    btree_insert(btree, 41, &d);
    btree_insert(btree, 70, &e);

    btree_erase(btree, 44);
    btree_erase(btree, 70);
    btree_erase(btree, 26);

    btree_insert(btree, 26, &a);
    btree_insert(btree, 70, &e);
    btree_insert(btree, 44, &b);

    ASSERT_THAT(*(int*)btree_find(btree, 26), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 41), Eq(d));
    ASSERT_THAT(*(int*)btree_find(btree, 44), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 70), Eq(e));
    ASSERT_THAT(*(int*)btree_find(btree, 82), Eq(c));

    btree_free(btree);
}

TEST(NAME, inserting_duplicate_keys_doesnt_replace_existing_elements)
{
    struct btree_t* btree; btree_create(&btree);

    int a=56, b=45, c=18;
    btree_insert(btree, 5, &a);
    btree_insert(btree, 3, &a);

    btree_insert(btree, 5, &b);
    btree_insert(btree, 4, &b);
    btree_insert(btree, 3, &c);

    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 4), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 5), Eq(a));

    btree_free(btree);
}

TEST(NAME, generating_keys_do_not_conflict_with_existing_ascending_keys)
{
    hash32_t key;
    struct btree_t* btree; btree_create(&btree);
    btree_insert(btree, 0, NULL);
    btree_insert(btree, 1, NULL);
    btree_insert(btree, 2, NULL);
    btree_insert(btree, 3, NULL);
    btree_insert(btree, 5, NULL);
    key = btree_find_unused_key(btree);
    ASSERT_NE(0, key);
    ASSERT_NE(1, key);
    ASSERT_NE(2, key);
    ASSERT_NE(3, key);
    ASSERT_NE(5, key);
    btree_free(btree);
}

TEST(NAME, generating_keys_do_not_conflict_with_existing_descending_keys)
{
    hash32_t key;
    struct btree_t* btree; btree_create(&btree);
    btree_insert(btree, 5, NULL);
    btree_insert(btree, 3, NULL);
    btree_insert(btree, 2, NULL);
    btree_insert(btree, 1, NULL);
    btree_insert(btree, 0, NULL);
    key = btree_find_unused_key(btree);
    ASSERT_NE(0, key);
    ASSERT_NE(1, key);
    ASSERT_NE(2, key);
    ASSERT_NE(3, key);
    ASSERT_NE(5, key);
    btree_free(btree);
}

TEST(NAME, generating_keys_do_not_conflict_with_existing_random_keys)
{
    hash32_t key;
    struct btree_t* btree; btree_create(&btree);
    btree_insert(btree, 2387, NULL);
    btree_insert(btree, 28, NULL);
    btree_insert(btree, 358, NULL);
    btree_insert(btree, 183, NULL);
    btree_insert(btree, 38, NULL);
    key = btree_find_unused_key(btree);
    ASSERT_NE(2387, key);
    ASSERT_NE(28, key);
    ASSERT_NE(358, key);
    ASSERT_NE(183, key);
    ASSERT_NE(38, key);
    btree_free(btree);
}

TEST(NAME, find_element)
{
    struct btree_t* btree; btree_create(&btree);
    int a = 6;
    btree_insert(btree, 2387, NULL);
    btree_insert(btree, 28, &a);
    btree_insert(btree, 358, NULL);
    btree_insert(btree, 183, NULL);
    btree_insert(btree, 38, NULL);

    hash32_t* key;
    ASSERT_THAT((key = btree_find_key(btree, &a)), NotNull());
    EXPECT_THAT(*key, Eq(28u));

    btree_free(btree);
}

TEST(NAME, set_value)
{
    struct btree_t* btree; btree_create(&btree);
    int a = 6;
    btree_insert(btree, 2387, NULL);
    btree_insert(btree, 28, NULL);
    btree_insert(btree, 358, NULL);
    btree_insert(btree, 183, NULL);
    btree_insert(btree, 38, NULL);

    btree_set(btree, 28, &a);

    EXPECT_THAT((int*)btree_find(btree, 28), Pointee(a));

    btree_free(btree);
}

TEST(NAME, get_any_element)
{
    struct btree_t* btree; btree_create(&btree);
    int a = 6;

    EXPECT_THAT(btree_get_any_element(btree), IsNull());
    btree_insert(btree, 45, &a);
    EXPECT_THAT(btree_get_any_element(btree), NotNull());
    btree_erase(btree, 45);
    EXPECT_THAT(btree_get_any_element(btree), IsNull());

    btree_free(btree);
}

TEST(NAME, key_exists)
{
    struct btree_t* btree; btree_create(&btree);

    EXPECT_THAT(btree_key_exists(btree, 29), Eq(-1));
    btree_insert(btree, 29, NULL);
    EXPECT_THAT(btree_key_exists(btree, 29), Eq(0));
    EXPECT_THAT(btree_key_exists(btree, 40), Eq(-1));
    btree_erase(btree, 29);
    EXPECT_THAT(btree_key_exists(btree, 29), Eq(-1));

    btree_free(btree);
}

TEST(NAME, erase_element)
{
    struct btree_t* btree; btree_create(&btree);
    int a = 6;

    EXPECT_THAT(btree_erase_value(btree, &a), IsNull());
    EXPECT_THAT(btree_erase_value(btree, NULL), IsNull());
    btree_insert(btree, 39, &a);
    EXPECT_THAT((int*)btree_erase_value(btree, &a), Pointee(a));

    btree_free(btree);
}

TEST(NAME, iterate_with_no_items)
{
    struct btree_t* btree; btree_create(&btree);
    {
        int counter = 0;
        BTREE_FOR_EACH(btree, int, key, value)
            ++counter;
        BTREE_END_EACH
        ASSERT_THAT(counter, Eq(0));
    }
    btree_free(btree);
}

TEST(NAME, iterate_5_random_items)
{
    struct btree_t* btree; btree_create(&btree);

    int a=79579, b=235, c=347, d=124, e=457;
    btree_insert(btree, 243, &a);
    btree_insert(btree, 256, &b);
    btree_insert(btree, 456, &c);
    btree_insert(btree, 468, &d);
    btree_insert(btree, 969, &e);

    int counter = 0;
    BTREE_FOR_EACH(btree, int, key, value)
        switch(counter)
        {
            case 0 : ASSERT_THAT(key, Eq(243u)); ASSERT_THAT(a, Eq(*value)); break;
            case 1 : ASSERT_THAT(key, Eq(256u)); ASSERT_THAT(b, Eq(*value)); break;
            case 2 : ASSERT_THAT(key, Eq(456u)); ASSERT_THAT(c, Eq(*value)); break;
            case 3 : ASSERT_THAT(key, Eq(468u)); ASSERT_THAT(d, Eq(*value)); break;
            case 4 : ASSERT_THAT(key, Eq(969u)); ASSERT_THAT(e, Eq(*value)); break;
            default: ASSERT_THAT(1, Eq(0)); break;
        }
        ++counter;
    BTREE_END_EACH
    ASSERT_THAT(counter, Eq(5));

    btree_free(btree);
}

TEST(NAME, iterate_5_null_items)
{
    struct btree_t* btree; btree_create(&btree);

    btree_insert(btree, 243, NULL);
    btree_insert(btree, 256, NULL);
    btree_insert(btree, 456, NULL);
    btree_insert(btree, 468, NULL);
    btree_insert(btree, 969, NULL);

    int counter = 0;
    BTREE_FOR_EACH(btree, int, key, value)
        switch(counter)
        {
            case 0 : ASSERT_THAT(key, Eq(243u)); ASSERT_THAT(value, IsNull()); break;
            case 1 : ASSERT_THAT(key, Eq(256u)); ASSERT_THAT(value, IsNull()); break;
            case 2 : ASSERT_THAT(key, Eq(456u)); ASSERT_THAT(value, IsNull()); break;
            case 3 : ASSERT_THAT(key, Eq(468u)); ASSERT_THAT(value, IsNull()); break;
            case 4 : ASSERT_THAT(key, Eq(969u)); ASSERT_THAT(value, IsNull()); break;
            default: ASSERT_THAT(1, Eq(0)); break;
        }
        ++counter;
    BTREE_END_EACH
    ASSERT_THAT(counter, Eq(5));

    btree_free(btree);
}

TEST(NAME, erase_in_for_loop)
{
    struct btree_t* btree; btree_create(&btree);

    int a=79579, b=235, c=347, d=124, e=457;
    btree_insert(btree, 243, &a);
    btree_insert(btree, 256, &b);
    btree_insert(btree, 456, &c);
    btree_insert(btree, 468, &d);
    btree_insert(btree, 969, &e);

    BTREE_FOR_EACH(btree, int, key, value)
        if(key == 256u)
            BTREE_ERASE_CURRENT_ITEM_IN_FOR_LOOP(btree, value);
    BTREE_END_EACH

    EXPECT_THAT((int*)btree_find(btree, 243), Pointee(a));
    EXPECT_THAT((int*)btree_find(btree, 256), IsNull());
    EXPECT_THAT((int*)btree_find(btree, 456), Pointee(c));
    EXPECT_THAT((int*)btree_find(btree, 468), Pointee(d));
    EXPECT_THAT((int*)btree_find(btree, 969), Pointee(e));

    btree_free(btree);
}
*/
