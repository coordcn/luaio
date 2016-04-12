/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio_pmemory.h"

typedef struct {
  luaio_list_t  free_list;
  uint32_t      max_free_chunks;
  uint32_t      free_chunks;
  uint32_t      size;
  uint32_t      luaio;
} luaio_pmemory_pool_t;

#define LUAIO_PMEMORY_MAX_SLOT  64

static luaio_pmemory_pool_t luaio_pmemory_pool[LUAIO_PMEMORY_MAX_SLOT];

static const uint32_t max_free_chunks[LUAIO_PMEMORY_MAX_SLOT] = {
  16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
  8192,  8192,  8192,  8192,  8192,  8192,  8192,  8192,
  4096,  4096,  4096,  4096,  4096,  4096,  4096,  4096,
  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,
  1024,  1024,  1024,  1024,  1024,  1024,  1024,  1024,
  512,   512,   512,   512,   512,   512,   512,   512,
  256,   256,   256,   256,   256,   256,   256,   256,
  128,   128,   128,   128,   128,   128,   128,   128
};

static const uint32_t size_table[LUAIO_PMEMORY_MAX_SLOT] = {
  /*step = 64*/
  64,    128,   192,   256,   320,   384,   448,   512,
  /*step = 64*/
  576,   640,   704,   768,   832,   896,   960,   1024,
  /*step = 128*/
  1152,  1280,  1408,  1536,  1664,  1792,  1920,  2048,
  /*step = 256*/
  2304,  2560,  2816,  3072,  3328,  3584,  3840,  4096,
  /*step = 512*/
  4608,  5120,  5632,  6144,  6656,  7168,  7680,  8192,
  /*step = 1024*/
  9216,  10240, 11264, 12288, 13312, 14336, 15360, 16384,
  /*step = 2048*/
  18432, 20480, 22528, 24576, 26624, 28672, 30720, 32768,
  /*step = 4096*/
  36864, 40960, 45056, 49152, 53248, 57344, 61440, 65536
};

#define LUAIO_PMEMORY_SMALL_CHUNK_ALIGNMENT   64
#define LUAIO_PMEMORY_SMALL_CHUNK_SHIFT       LUAIO_64_SHIFT
#define LUAIO_PMEMORY_MAX_SMALL_CHUNK_SIZE    (4 * 1024)

#define LUAIO_PMEMORY_LARGE_CHUNK_ALIGNMENT   512
#define LUAIO_PMEMORY_LARGE_CHUNK_SHIFT       LUAIO_512_SHIFT
#define LUAIO_PMEMORY_MAX_LARGE_CHUNK_SIZE    (64 * 1024)

static const uint8_t indexes_small[64] = {
  /*step = 64*/
  0, 1, 2, 3, 4, 5, 6, 7,
  /*step = 64*/
  8, 9, 10, 11, 12, 13, 14, 15,
  /*step = 128*/
  16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23,
  /*step = 256*/
  24, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 27,
  28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31, 31
};

static const uint8_t indexes_large[128] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  /*step = 512*/
  32, 33, 34, 35, 36, 37, 38, 39,
  /*step = 1024*/
  40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47,
  /*step = 2048*/
  48, 48, 48, 48, 49, 49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 51,
  52, 52, 52, 52, 53, 53, 53, 53, 54, 54, 54, 54, 55, 55, 55, 55,
  /*step = 4096*/
  56, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 57,
  58, 58, 58, 58, 58, 58, 58, 58, 59, 59, 59, 59, 59, 59, 59, 59,
  60, 60, 60, 60, 60, 60, 60, 60, 61, 61, 61, 61, 61, 61, 61, 61,
  62, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63
};

void luaio_pmemory_init() {
  luaio_pmemory_pool_t *pool;
  for (size_t i = 0; i < LUAIO_PMEMORY_MAX_SLOT; i++) {
    pool = &luaio_pmemory_pool[i];
    luaio_list_init(&pool->free_list);
    pool->max_free_chunks = max_free_chunks[i];
    pool->free_chunks = 0;
    pool->size = size_table[i];
    pool->luaio = 0;
  }
}

static void *luaio_pmemory_alloc(luaio_pmemory_pool_t *pool, size_t used) {
  luaio_list_t *free_list = &pool->free_list;
  size_t size = pool->size;
  luaio_pmemory_chunk_t *chunk;

  if (!luaio_list_is_empty(free_list)) {
    luaio_list_t *list = free_list->next;
    chunk = luaio_list_entry(list, luaio_pmemory_chunk_t, list);
    luaio_list_remove(list);
    pool->free_chunks--;  
  } else {
    chunk = luaio_memalign(LUAIO_PMEMORY_ALIGNMENT, size + sizeof(luaio_pmemory_chunk_t));
    if (chunk == NULL) return NULL;
  }

  chunk->cookie.size = size;
  chunk->cookie.used = used;
  return (void*)((char*)chunk + sizeof(luaio_pmemory_chunk_t));
}

void *luaio__palloc(size_t size) {
  uint8_t index;
  size_t aligned_size;
  if (size <= LUAIO_PMEMORY_MAX_SMALL_CHUNK_SIZE) {
    aligned_size = luaio_align(size, LUAIO_PMEMORY_SMALL_CHUNK_ALIGNMENT);
    index = indexes_small[(aligned_size >> LUAIO_PMEMORY_SMALL_CHUNK_SHIFT) - 1];
    return luaio_pmemory_alloc(&luaio_pmemory_pool[index], size);
  } else if (size <= LUAIO_PMEMORY_MAX_LARGE_CHUNK_SIZE) {
    aligned_size = luaio_align(size, LUAIO_PMEMORY_LARGE_CHUNK_ALIGNMENT);
    index = indexes_large[(aligned_size >> LUAIO_PMEMORY_LARGE_CHUNK_SHIFT) - 1];
    return luaio_pmemory_alloc(&luaio_pmemory_pool[index], size);
  } else {
    luaio_pmemory_chunk_t *chunk = luaio_memalign(LUAIO_PMEMORY_ALIGNMENT, size + sizeof(luaio_pmemory_chunk_t));
    if (chunk == NULL) return NULL;
    chunk->cookie.size = size;
    chunk->cookie.used = size;
    return (void*)((char*)chunk + sizeof(luaio_pmemory_chunk_t));
  }
}

void *luaio__prealloc(void *p, size_t size) {
  if (size == 0 && p != NULL) {
    luaio__pfree(p);
    return NULL;
  }

  if (p == NULL) return luaio__palloc(size);

  luaio_pmemory_chunk_t *old_chunk = (luaio_pmemory_chunk_t*)((char*)p - sizeof(luaio_pmemory_chunk_t));
  size_t old_size = old_chunk->cookie.size;
  size_t old_used = old_chunk->cookie.used;

  if (size <= old_size) {
    old_chunk->cookie.used = size;
    return p;
  }

  void *new_p = luaio__palloc(size);
  if (new_p == NULL) return NULL;
  luaio_memcpy(new_p, p, old_used);
  luaio__pfree(p);

  return new_p;
}

void luaio__pfree(void *p) {
  if (p != NULL) {
    luaio_pmemory_chunk_t *chunk = (luaio_pmemory_chunk_t*)((char*)p - sizeof(luaio_pmemory_chunk_t));

    uint8_t index;
    /*size_t aligned_size;*/
    luaio_pmemory_pool_t *pool;
    size_t size = chunk->cookie.size;

    if (size <= LUAIO_PMEMORY_MAX_SMALL_CHUNK_SIZE) {
      /*aligned_size = luaio_align(size, LUAIO_PMEMORY_SMALL_CHUNK_ALIGNMENT);*/
      index = indexes_small[(size >> LUAIO_PMEMORY_SMALL_CHUNK_SHIFT) - 1];
      pool = &luaio_pmemory_pool[index];
      if (pool->free_chunks < pool->max_free_chunks) {
        luaio_list_insert_head(&chunk->list, &pool->free_list);
        pool->free_chunks++;
        return;
      }
    } else if (size <= LUAIO_PMEMORY_MAX_LARGE_CHUNK_SIZE) {
      /*aligned_size = luaio_align(size, LUAIO_PMEMORY_LARGE_CHUNK_ALIGNMENT);*/
      index = indexes_large[(size >> LUAIO_PMEMORY_LARGE_CHUNK_SHIFT) - 1];
      pool = &luaio_pmemory_pool[index];
      if (pool->free_chunks < pool->max_free_chunks) {
        luaio_list_insert_head(&chunk->list, &pool->free_list);
        pool->free_chunks++;
        return;
      }
    }

    luaio_free(chunk);
  }
}
