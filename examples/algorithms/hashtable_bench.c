#define BENCHMARK_IMPLEMENTATION
#include "benchmark_single.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TABLE_SIZE 1024
#define NUM_OPS    100

static uint32_t hash_djb2(const char *s) { uint32_t h = 5381; while (*s) h = ((h << 5) + h) + *s++; return h; }
static uint32_t hash_fnv1a(const char *s) { uint32_t h = 2166136261u; while (*s) { h ^= (uint8_t)*s++; h *= 16777619u; } return h; }
static uint32_t hash_simple(const char *s) { uint32_t h = 0; while (*s) h += *s++; return h; }

typedef struct { char *keys[TABLE_SIZE]; int values[TABLE_SIZE]; uint32_t (*hash)(const char*); } ht_open_t;

static void ht_open_init(ht_open_t *ht, uint32_t (*h)(const char*)) { memset(ht->keys, 0, sizeof(ht->keys)); ht->hash = h; }
static void ht_open_insert(ht_open_t *ht, const char *key, int val) {
    uint32_t i = ht->hash(key) % TABLE_SIZE;
    while (ht->keys[i]) i = (i + 1) % TABLE_SIZE;
    ht->keys[i] = (char*)key; ht->values[i] = val;
}
static int ht_open_get(ht_open_t *ht, const char *key) {
    uint32_t i = ht->hash(key) % TABLE_SIZE;
    while (ht->keys[i]) { if (strcmp(ht->keys[i], key) == 0) return ht->values[i]; i = (i + 1) % TABLE_SIZE; }
    return -1;
}

typedef struct node { char *key; int val; struct node *next; } node_t;
typedef struct { node_t *buckets[TABLE_SIZE]; node_t pool[NUM_OPS]; int idx; uint32_t (*hash)(const char*); } ht_chain_t;

static void ht_chain_init(ht_chain_t *ht, uint32_t (*h)(const char*)) { memset(ht->buckets, 0, sizeof(ht->buckets)); ht->idx = 0; ht->hash = h; }
static void ht_chain_insert(ht_chain_t *ht, const char *key, int val) {
    uint32_t i = ht->hash(key) % TABLE_SIZE;
    node_t *n = &ht->pool[ht->idx++]; n->key = (char*)key; n->val = val; n->next = ht->buckets[i]; ht->buckets[i] = n;
}
static int ht_chain_get(ht_chain_t *ht, const char *key) {
    for (node_t *n = ht->buckets[ht->hash(key) % TABLE_SIZE]; n; n = n->next) if (strcmp(n->key, key) == 0) return n->val;
    return -1;
}

static const char *keys[] = {"apple","banana","cherry","date","elderberry","fig","grape","honeydew","kiwi","lemon",
    "mango","nectarine","orange","papaya","quince","raspberry","strawberry","tangerine","ugli","vanilla"};
#define NUM_KEYS (sizeof(keys)/sizeof(keys[0]))

static ht_open_t g_open;
static ht_chain_t g_chain;

BENCH(open_djb2_ins, "open+djb2 ins") { ht_open_init(&g_open, hash_djb2); for (int i = 0; i < NUM_OPS; i++) ht_open_insert(&g_open, keys[i % NUM_KEYS], i); KEEP(g_open); }
BENCH(open_fnv1a_ins, "open+fnv1a ins") { ht_open_init(&g_open, hash_fnv1a); for (int i = 0; i < NUM_OPS; i++) ht_open_insert(&g_open, keys[i % NUM_KEYS], i); KEEP(g_open); }
BENCH(open_simple_ins, "open+simple ins") { ht_open_init(&g_open, hash_simple); for (int i = 0; i < NUM_OPS; i++) ht_open_insert(&g_open, keys[i % NUM_KEYS], i); KEEP(g_open); }
BENCH(chain_djb2_ins, "chain+djb2 ins") { ht_chain_init(&g_chain, hash_djb2); for (int i = 0; i < NUM_OPS; i++) ht_chain_insert(&g_chain, keys[i % NUM_KEYS], i); KEEP(g_chain); }
BENCH(chain_fnv1a_ins, "chain+fnv1a ins") { ht_chain_init(&g_chain, hash_fnv1a); for (int i = 0; i < NUM_OPS; i++) ht_chain_insert(&g_chain, keys[i % NUM_KEYS], i); KEEP(g_chain); }

BENCH(open_djb2_get, "open+djb2 get") {
    ht_open_init(&g_open, hash_djb2); for (int i = 0; i < (int)NUM_KEYS; i++) ht_open_insert(&g_open, keys[i], i);
    volatile int s = 0; for (int i = 0; i < NUM_OPS; i++) s += ht_open_get(&g_open, keys[i % NUM_KEYS]); KEEP(s);
}
BENCH(chain_djb2_get, "chain+djb2 get") {
    ht_chain_init(&g_chain, hash_djb2); for (int i = 0; i < (int)NUM_KEYS; i++) ht_chain_insert(&g_chain, keys[i], i);
    volatile int s = 0; for (int i = 0; i < NUM_OPS; i++) s += ht_chain_get(&g_chain, keys[i % NUM_KEYS]); KEEP(s);
}

int main(void) { return bench_main(); }
