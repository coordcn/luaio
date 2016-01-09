/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "palloc.h"

void LuaIO_pool_init(LuaIO_pool_t* pool, size_t max_free_chunks) {
  LuaIO_list_init(&pool->free_list);
  pool->max_free_chunks = max_free_chunks;
  pool->free_chunks = 0;
}

void* LuaIO_pool_alloc(LuaIO_pool_t* pool, size_t size, size_t align, size_t slot) {
  LuaIO_list_t* free_list = &pool->free_list;
  LuaIO_pool_chunk_t* chunk;

  if (!LuaIO_list_is_empty(free_list)) {
    LuaIO_list_t* list = free_list->next;
    chunk = LuaIO_list_entry(list, LuaIO_pool_chunk_t, list);
    LuaIO_list_remove(list);
    pool->free_chunks--;  
  } else {
    chunk = LuaIO_memalign(align, size + sizeof(LuaIO_pool_chunk_t));
    if (chunk == NULL) return NULL;
  }

  chunk->magic = slot;
  return (void*)((char*)chunk + sizeof(LuaIO_pool_chunk_t));
}

void LuaIO_pool_free(LuaIO_pool_t* pool, void* p) {
  if (p != NULL) {
    LuaIO_pool_chunk_t* chunk = (LuaIO_pool_chunk_t*)((char*)p - sizeof(LuaIO_pool_chunk_t));
    
    if (pool->free_chunks < pool->max_free_chunks) {
      LuaIO_list_insert_head(&chunk->list, &pool->free_list);
      pool->free_chunks++;
    } else {
      LuaIO_free(chunk);
    }
  }
}
