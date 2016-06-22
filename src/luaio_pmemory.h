/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_PMEMORY_H
#define LUAIO_PMEMORY_H

#include "luaio_list.h"
#include "luaio_config.h"

#define LUAIO_PMEMORY_ALIGNMENT sizeof(void*)

typedef struct {
  size_t capacity;
  size_t size;      /* string length */
} luaio_pmemory_cookie_t;

typedef union {
  luaio_list_t              list;
  luaio_pmemory_cookie_t    cookie;
} luaio_pmemory_chunk_t;

void luaio_pmemory_init();

void *luaio_palloc(size_t size);
void luaio_pfree(void *p);
void *luaio_prealloc(void *p, size_t size);

static inline size_t luaio_pmemory_get_capacity(void* p) {
  luaio_pmemory_chunk_t *chunk = (luaio_pmemory_chunk_t*)((char*)p - sizeof(luaio_pmemory_chunk_t));
  return chunk->cookie.capacity;
}

static inline size_t luaio_pmemory_get_size(void* p) {
  luaio_pmemory_chunk_t *chunk = (luaio_pmemory_chunk_t*)((char*)p - sizeof(luaio_pmemory_chunk_t));
  return chunk->cookie.size;
}

static inline void luaio_pmemory_set_size(void* p, size_t size) {
  luaio_pmemory_chunk_t *chunk = (luaio_pmemory_chunk_t*)((char*)p - sizeof(luaio_pmemory_chunk_t));
  chunk->cookie.size = size;
}

#endif /* LUAIO_PMEMORY_H */
