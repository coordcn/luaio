/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_PMEMORY_H
#define LUAIO_PMEMORY_H

#include "luaio_list.h"
#include "luaio_config.h"

#define LUAIO_PMEMORY_ALIGNMENT sizeof(size_t)

typedef struct {
  size_t size;
  size_t used;
} luaio_pmemory_cookie_t;

typedef union {
  luaio_list_t              list;
  luaio_pmemory_cookie_t    cookie;
} luaio_pmemory_chunk_t;

void luaio_pmemory_init();

void *luaio__palloc(size_t size);
void *luaio__prealloc(void *p, size_t size);
void luaio__pfree(void *p);

static inline size_t luaio__pmemory_get_size(void* p) {
  luaio_pmemory_chunk_t *chunk = (luaio_pmemory_chunk_t*)((char*)p - sizeof(luaio_pmemory_chunk_t));
  return chunk->cookie.size;
}

static inline size_t luaio__pmemory_get_used(void* p) {
  luaio_pmemory_chunk_t *chunk = (luaio_pmemory_chunk_t*)((char*)p - sizeof(luaio_pmemory_chunk_t));
  return chunk->cookie.used;
}

static inline void luaio__pmemory_set_used(void* p, size_t used) {
  luaio_pmemory_chunk_t *chunk = (luaio_pmemory_chunk_t*)((char*)p - sizeof(luaio_pmemory_chunk_t));
  chunk->cookie.used = used;
}

#ifdef LUAIO_USE_PMEMORY

#define luaio_palloc(size)                luaio__palloc(size)
#define luaio_prealloc(p, size)           luaio__prealloc(p, size)
#define luaio_pfree(p)                    luaio__pfree(p)
#define luaio_pmemory_size(p, size)       luaio__pmemory_get_size(p)
#define luaio_pmemory_set_used(p, size)   luaio__pmemory_set_used(p, size)

#else

#define luaio_palloc(size)                luaio_malloc(size)
#define luaio_prealloc(p, size)           luaio_realloc(p, size)
#define luaio_pfree(p)                    luaio_free(p)
#define luaio_pmemory_size(p, size)       (size)
#define luaio_pmemory_set_used(p, size)   

#endif

#endif /* LUAIO_PMEMORY_H */
