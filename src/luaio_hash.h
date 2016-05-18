/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_HASH_H
#define LUAIO_HASH_H

#include "luaio_list.h"
#include "luaio_config.h"
#include "luaio_pmemory.h"
#include "luaio_string.h"

#if LUAIO_BITS == 16
#define luaio_hash_slot    luaio_hash_slot16
#elif LUAIO_BITS == 32
#define luaio_hash_slot    luaio_hash_slot32
#elif LUAIO_BITS == 64
#define luaio_hash_slot    luaio_hash_slot64
#else
#error LUAIO_BITS not 64 or 32 or 16
#endif

#define luaio_hash         luaio_hash_DJB
#define luaio_hash_lower   luaio_hash_DJB_lower

static inline size_t luaio_hash_DJB(const char *str, size_t n) {
  size_t hash = 5381;

  for (size_t i = 0; i < n; i++) {
    hash += (hash << 5) + str[i];
  }

  return hash;
}

static inline size_t luaio_hash_DJB_lower(const char *str, size_t n) {
  size_t hash = 5381;
  
  for (size_t i = 0; i < n; i++) {
    hash += (hash << 5) + luaio_lower(str[i]);
  }

  return hash;
}

#define LUAIO_GOLDEN_RATIO_PRIME16       40503UL
#define LUAIO_GOLDEN_RATIO_PRIME32       2654435769UL
#define LUAIO_GOLDEN_RATIO_PRIME64       11400714819323198485UL

/* if slot size is 1024, used_bits = 10
 * bits = 64 - used_bits;
 */
static inline size_t luaio_hash_slot16(size_t value, size_t bits) {
  return (value * LUAIO_GOLDEN_RATIO_PRIME16) >> (16 - bits);
}

/* if slot size is 1024, used_bits = 10
 * bits = 64 - used_bits;
 */
static inline size_t luaio_hash_slot32(size_t value, size_t bits) {
  return (value * LUAIO_GOLDEN_RATIO_PRIME32) >> (32 - bits);
}

/* if slot size is 1024, used_bits = 10
 * bits = 64 - used_bits;
 * linux kernel hash.h is better
 */
static inline size_t luaio_hash_slot64(size_t value, size_t bits) {
  return (value * LUAIO_GOLDEN_RATIO_PRIME64) >> (64 - bits);
}

typedef int (*luaio_hash_strcmp_fn)(const char *s1, const char *s2, size_t n);
typedef size_t (*luaio_hash_fn)(const char *s, size_t n);

typedef struct {
  luaio_hlist_head_t    *slots;
  size_t                max_items;
  size_t                max_age;
  size_t                items;
  size_t                bits;
  luaio_hash_strcmp_fn  str_cmp;
  luaio_hash_fn         hash;
} luaio_hash_t;

typedef struct {
  luaio_hlist_node_t  node;
  void                *pointer;
  size_t              hash;
  size_t              expires;
  size_t              key_length;
  char                *key;
  int                 value;
} luaio_hash_item_t;

luaio_hash_t *luaio_hash__create(size_t bits, 
                                 size_t max_age, 
                                 luaio_hash_strcmp_fn strcmp_fn, 
                                 luaio_hash_fn hash_fn);

#define luaio_hash_create_int_pointer(bits, max_age) \
  luaio_hash__create(bits, max_age, NULL, NULL)

#define luaio_hash_create_int_value(bits) \
  luaio_hash__create(bits, 0, NULL, NULL)

#define luaio_hash_create_str_pointer(bits, max_age, free) \
  luaio_hash__create(bits, max_age, luaio_strncmp, luaio_hash)

#define luaio_hash_create_strcase_pointer(bits, max_age, free) \
  luaio_hash__create(bits, max_age, luaio_strncasecmp, luaio_hash_lower)

void luaio_hash_destroy(luaio_hash_t *hash);

void luaio_hash_str_set(luaio_hash_t *hash, char *key, size_t n, void *pointer);
void *luaio_hash_str_get(luaio_hash_t *hash, const char *key, size_t n);
void luaio_hash_str_remove(luaio_hash_t *hash, const char *key, size_t n);

void luaio_hash_int_set(luaio_hash_t *hash, size_t key, void *pointer);
void *luaio_hash_int_get(luaio_hash_t *hash, size_t key);
void luaio_hash_int_remove(luaio_hash_t *hash, size_t key);

/*just for thread -> thread_ref*/
void luaio_hash_int_set_value(luaio_hash_t *hash, size_t key, int value);
int luaio_hash_int_get_value(luaio_hash_t *hash, size_t key, int *value);
void luaio_hash_int_remove_value(luaio_hash_t *hash, size_t key);
int luaio_hash_int_get_and_remove_value(luaio_hash_t *hash, size_t key, int *value);

#endif /* LUAIO_HASH_H */
