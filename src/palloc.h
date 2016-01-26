/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: allocate same size memory by memory pool
 */

#ifndef LUAIO_PALLOC_H
#define LUAIO_PALLOC_H

#include "list.h"
#include "alloc.h"

#define LUAIO_POOL_ALIGNMENT              sizeof(unsigned long)
#define LUAIO_POOL_MAGIC                  0x55AA55AA

#define LuaIO_palloc(pool, size) LuaIO_pool_alloc(pool, size, LUAIO_POOL_ALIGNMENT, LUAIO_POOL_MAGIC)
#define LuaIO_pfree(pool, p) LuaIO_pool_free(pool, p)

typedef struct {
  LuaIO_list_t  free_list;
  size_t        max_free_chunks;
  size_t        free_chunks;
} LuaIO_pool_t;

typedef union {
  LuaIO_list_t  list;
  size_t        magic;
} LuaIO_pool_chunk_t;

void LuaIO_pool_init(LuaIO_pool_t *pool, size_t max_free_chunks);
static inline void LuaIO_pool_set_max_free_chunks(LuaIO_pool_t *pool, 
                                                  size_t max_free_chunks) {
  pool->max_free_chunks = max_free_chunks;
}

void *LuaIO_pool_alloc(LuaIO_pool_t *pool, size_t size, size_t align, size_t slot);
void LuaIO_pool_free(LuaIO_pool_t *pool, void* p);

#endif /* LUAIO_PALLOC_H */
