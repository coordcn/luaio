/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_HASH_H
#define LUAIO_HASH_H

#include "list.h"
#include "alloc.h"
#include "palloc.h"
#include "config.h"

#if LUAIO_BITS == 16
#define LuaIO_hash_slot    LuaIO_hash_slot16
#elif LUAIO_BITS == 32
#define LuaIO_hash_slot    LuaIO_hash_slot32
#elif LUAIO_BITS == 64
#define LuaIO_hash_slot    LuaIO_hash_slot64
#else
#error LUAIO_BITS not 16 or 32 or 64
#endif

#define LuaIO_hash         LuaIO_hash_DJB
#define LuaIO_hash_lower   LuaIO_hash_DJB_lower

static inline size_t LuaIO_hash_DJB(const char *str, size_t n) {
  size_t hash = 5381;

  for (size_t i = 0; i < n; i++) {
    hash += (hash << 5) + str[i];
  }

  return hash;
}

static inline size_t LuaIO_hash_DJB_lower(const char *str, size_t n) {
  size_t hash = 5381;
  
  for (size_t i = 0; i < n; i++) {
    hash += (hash << 5) + LuaIO_lower(str[i]);
  }

  return hash;
}

#define LUAIO_GOLDEN_RATIO_PRIME16       40503UL
#define LUAIO_GOLDEN_RATIO_PRIME32       2654435769UL
#define LUAIO_GOLDEN_RATIO_PRIME64       11400714819323198485UL

/* if slot size is 1024, used_bits = 10
 * bits = 64 - used_bits;
 */
static inline size_t LuaIO_hash_slot16(size_t value, size_t bits) {
  return (value * LUAIO_GOLDEN_RATIO_PRIME16) >> (16 - bits);
}

/* if slot size is 1024, used_bits = 10
 * bits = 64 - used_bits;
 */
static inline size_t LuaIO_hash_slot32(size_t value, size_t bits) {
  return (value * LUAIO_GOLDEN_RATIO_PRIME32) >> (32 - bits);
}

/* if slot size is 1024, used_bits = 10
 * bits = 64 - used_bits;
 * linux kernel hash.h is better
 */
static inline size_t LuaIO_hash_slot64(size_t value, size_t bits) {
  return (value * LUAIO_GOLDEN_RATIO_PRIME64) >> (64 - bits);
}

typedef void (*LuaIO_hash_free_fn)(void *p);
typedef int (*LuaIO_hash_strcmp_fn)(const char *s1, const char *s2, size_t n);
typedef size_t (*LuaIO_hash_fn)(const char *s, size_t n);

typedef struct {
  size_t                max_items;
  size_t                max_age;
  size_t                items;
  size_t                bits;
  LuaIO_hash_free_fn    free;
  LuaIO_hash_strcmp_fn  str_cmp;
  LuaIO_hash_fn         hash;
  LuaIO_hlist_head_t    *slots;
} LuaIO_hash_t;

typedef struct {
  LuaIO_hlist_node_t  node;
  void                *pointer;
  size_t              hash;
  size_t              expires;
  size_t              key_length;
  char                *key;
  int                 value;
} LuaIO_hash_item_t;

LuaIO_hash_t *LuaIO_hash__create(size_t bits, 
                                 size_t max_age, 
                                 LuaIO_hash_free_fn free_fn, 
                                 LuaIO_hash_strcmp_fn strcmp_fn, 
                                 LuaIO_hash_fn hash_fn);

#define LuaIO_hash_create_int_pointer(bits, max_age, free) \
  LuaIO_hash__create(bits, max_age, free, NULL, NULL)

#define LuaIO_hash_create_int_value(bits) \
  LuaIO_hash__create(bits, 0, NULL, NULL, NULL)

#define LuaIO_hash_create_str_pointer(bits, max_age, free) \
  LuaIO_hash__create(bits, max_age, free, LuaIO_strncmp, LuaIO_hash)

#define LuaIO_hash_create_strcase_pointer(bits, max_age, free) \
  LuaIO_hash__create(bits, max_age, free, LuaIO_strncasecmp, LuaIO_hash_lower)

void LuaIO_hash_destroy(LuaIO_hash_t *hash);

void LuaIO_hash_str_set(LuaIO_hash_t *hash, char *key, size_t n, void *pointer);
void *LuaIO_hash_str_get(LuaIO_hash_t *hash, const char *key, size_t n);
void LuaIO_hash_str_remove(LuaIO_hash_t *hash, const char *key, size_t n);

void LuaIO_hash_int_set(LuaIO_hash_t *hash, size_t key, void *pointer);
void *LuaIO_hash_int_get(LuaIO_hash_t *hash, size_t key);
void LuaIO_hash_int_remove(LuaIO_hash_t *hash, size_t key);

/*just for thread -> thread_ref*/
void LuaIO_hash_int_set_value(LuaIO_hash_t *hash, size_t key, int value);
int LuaIO_hash_int_get_value(LuaIO_hash_t *hash, size_t key, int *value);
void LuaIO_hash_int_remove_value(LuaIO_hash_t *hash, size_t key);
int LuaIO_hash_int_get_and_remove_value(LuaIO_hash_t *hash, size_t key, int *value);

#endif /* LUAIO_HASH_H */
