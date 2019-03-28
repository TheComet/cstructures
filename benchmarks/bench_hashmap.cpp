#include "benchmark/benchmark.h"
#include "cstructures/hashmap.h"
#include "cstructures/hash.h"
#include <iostream>
#include <random>
#include <unordered_map>
#include <string.h>
#include <vector>

using namespace benchmark;

static std::mt19937 rng;

static uint8_t randomChar()
{
    std::uniform_int_distribution<uint8_t> dist(0, 255);
    return dist(rng);
}

static void fillRandom(char* s, uint32_t len)
{
    while (len--)
        *s++ = randomChar();
}

static void BM_HashmapCreation(State& state)
{
    uint32_t key_size = state.range(0);
    uint32_t value_size = state.range(1);

    for (auto _ : state)
    {
        hashmap_t hm;
        hashmap_init(&hm, key_size, value_size);
        DoNotOptimize(hm.storage);
        hashmap_deinit(&hm);
    }
}
BENCHMARK(BM_HashmapCreation)
    //->RangeMultiplier(2)->Ranges({{1<<0, 1<<16}, {1<<0, 1<<16}})
    ->RangeMultiplier(64)->Ranges({{1<<4, 1<<8}, {1<<4, 1<<8}})
    ;

template <typename K, typename V>
static void BM_HashmapInsert(State& state)
{
    K key;
    V value;

    for (auto _ : state)
    {
        hashmap_t hm;
        hashmap_init(&hm, sizeof(K), sizeof(V));
        for (size_t i = 0; i != state.range(0); ++i)
        {
            memcpy(&key, &i, sizeof(uint32_t));
            memcpy(&value, &i, sizeof(uint32_t));
            hashmap_insert(&hm, &key, &value);
        }
        DoNotOptimize(hm.storage);
        ClobberMemory();
        hashmap_deinit(&hm);
    }
}

template <typename K, typename V>
static void BM_StdUnorderedMap(State& state)
{
    uint32_t insertions = state.range(0);
    std::vector<K> keys(insertions);
    std::vector<V> values(insertions);
    for (auto& key : keys)
        fillRandom((char*)&key, sizeof(K));
    for (char* value : values)
        fillRandom(value, sizeof(V));

    for (auto _ : state)
    {
        std::unordered_map<K, V> hm;
        for (size_t i = 0; i != insertions; ++i)
        {
            uint8_t key = keys[i];
            hm.emplace(key, values[i]);
        }
        ClobberMemory();
    }
}

#define BENCH_KV(bench, k, v) \
    BENCHMARK_TEMPLATE(bench, k, v)->RangeMultiplier(2)->Ranges({{1<<0, 1<<16}});

BENCH_KV(BM_HashmapInsert, uint8_t, char[1<<0]);
BENCH_KV(BM_HashmapInsert, uint8_t, char[1<<1]);
BENCH_KV(BM_HashmapInsert, uint8_t, char[1<<2]);
BENCH_KV(BM_HashmapInsert, uint8_t, char[1<<3]);
BENCH_KV(BM_HashmapInsert, uint8_t, char[1<<4]);
BENCH_KV(BM_HashmapInsert, uint8_t, char[1<<5]);
BENCH_KV(BM_HashmapInsert, uint8_t, char[1<<6]);
BENCH_KV(BM_HashmapInsert, uint8_t, char[1<<7]);
BENCH_KV(BM_HashmapInsert, uint8_t, char[1<<8]);
BENCH_KV(BM_HashmapInsert, uint16_t, char[1<<0]);
BENCH_KV(BM_HashmapInsert, uint16_t, char[1<<1]);
BENCH_KV(BM_HashmapInsert, uint16_t, char[1<<2]);
BENCH_KV(BM_HashmapInsert, uint16_t, char[1<<3]);
BENCH_KV(BM_HashmapInsert, uint16_t, char[1<<4]);
BENCH_KV(BM_HashmapInsert, uint16_t, char[1<<5]);
BENCH_KV(BM_HashmapInsert, uint16_t, char[1<<6]);
BENCH_KV(BM_HashmapInsert, uint16_t, char[1<<7]);
BENCH_KV(BM_HashmapInsert, uint16_t, char[1<<8]);
BENCH_KV(BM_HashmapInsert, uint32_t, char[1<<0]);
BENCH_KV(BM_HashmapInsert, uint32_t, char[1<<1]);
BENCH_KV(BM_HashmapInsert, uint32_t, char[1<<2]);
BENCH_KV(BM_HashmapInsert, uint32_t, char[1<<3]);
BENCH_KV(BM_HashmapInsert, uint32_t, char[1<<4]);
BENCH_KV(BM_HashmapInsert, uint32_t, char[1<<5]);
BENCH_KV(BM_HashmapInsert, uint32_t, char[1<<6]);
BENCH_KV(BM_HashmapInsert, uint32_t, char[1<<7]);
BENCH_KV(BM_HashmapInsert, uint32_t, char[1<<8]);
BENCH_KV(BM_HashmapInsert, uint64_t, char[1<<0]);
BENCH_KV(BM_HashmapInsert, uint64_t, char[1<<1]);
BENCH_KV(BM_HashmapInsert, uint64_t, char[1<<2]);
BENCH_KV(BM_HashmapInsert, uint64_t, char[1<<3]);
BENCH_KV(BM_HashmapInsert, uint64_t, char[1<<4]);
BENCH_KV(BM_HashmapInsert, uint64_t, char[1<<5]);
BENCH_KV(BM_HashmapInsert, uint64_t, char[1<<6]);
BENCH_KV(BM_HashmapInsert, uint64_t, char[1<<7]);
BENCH_KV(BM_HashmapInsert, uint64_t, char[1<<8]);


BENCH_KV(BM_StdUnorderedMap, uint8_t, char[1<<0]);
BENCH_KV(BM_StdUnorderedMap, uint8_t, char[1<<1]);
BENCH_KV(BM_StdUnorderedMap, uint8_t, char[1<<2]);
BENCH_KV(BM_StdUnorderedMap, uint8_t, char[1<<3]);
BENCH_KV(BM_StdUnorderedMap, uint8_t, char[1<<4]);
BENCH_KV(BM_StdUnorderedMap, uint8_t, char[1<<5]);
BENCH_KV(BM_StdUnorderedMap, uint8_t, char[1<<6]);
BENCH_KV(BM_StdUnorderedMap, uint8_t, char[1<<7]);
BENCH_KV(BM_StdUnorderedMap, uint8_t, char[1<<8]);
BENCH_KV(BM_StdUnorderedMap, uint16_t, char[1<<0]);
BENCH_KV(BM_StdUnorderedMap, uint16_t, char[1<<1]);
BENCH_KV(BM_StdUnorderedMap, uint16_t, char[1<<2]);
BENCH_KV(BM_StdUnorderedMap, uint16_t, char[1<<3]);
BENCH_KV(BM_StdUnorderedMap, uint16_t, char[1<<4]);
BENCH_KV(BM_StdUnorderedMap, uint16_t, char[1<<5]);
BENCH_KV(BM_StdUnorderedMap, uint16_t, char[1<<6]);
BENCH_KV(BM_StdUnorderedMap, uint16_t, char[1<<7]);
BENCH_KV(BM_StdUnorderedMap, uint16_t, char[1<<8]);
BENCH_KV(BM_StdUnorderedMap, uint32_t, char[1<<0]);
BENCH_KV(BM_StdUnorderedMap, uint32_t, char[1<<1]);
BENCH_KV(BM_StdUnorderedMap, uint32_t, char[1<<2]);
BENCH_KV(BM_StdUnorderedMap, uint32_t, char[1<<3]);
BENCH_KV(BM_StdUnorderedMap, uint32_t, char[1<<4]);
BENCH_KV(BM_StdUnorderedMap, uint32_t, char[1<<5]);
BENCH_KV(BM_StdUnorderedMap, uint32_t, char[1<<6]);
BENCH_KV(BM_StdUnorderedMap, uint32_t, char[1<<7]);
BENCH_KV(BM_StdUnorderedMap, uint32_t, char[1<<8]);
BENCH_KV(BM_StdUnorderedMap, uint64_t, char[1<<0]);
BENCH_KV(BM_StdUnorderedMap, uint64_t, char[1<<1]);
BENCH_KV(BM_StdUnorderedMap, uint64_t, char[1<<2]);
BENCH_KV(BM_StdUnorderedMap, uint64_t, char[1<<3]);
BENCH_KV(BM_StdUnorderedMap, uint64_t, char[1<<4]);
BENCH_KV(BM_StdUnorderedMap, uint64_t, char[1<<5]);
BENCH_KV(BM_StdUnorderedMap, uint64_t, char[1<<6]);
BENCH_KV(BM_StdUnorderedMap, uint64_t, char[1<<7]);
BENCH_KV(BM_StdUnorderedMap, uint64_t, char[1<<8]);
