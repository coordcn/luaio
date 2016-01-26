/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "pmemory.h"

#define LUAIO_PMEMORY_MAGIC 0xAA55AA55

static LuaIO_pool_t LuaIO_pmemory_slots[LUAIO_PMEMORY_SLOT_SIZE];

void LuaIO_pmemory_init(size_t max_free_chunks) {
  for (size_t i = 0; i < LUAIO_PMEMORY_SLOT_SIZE; i++) {
    LuaIO_pool_init(&LuaIO_pmemory_slots[i], max_free_chunks);
  }
}

void LuaIO_pmemory_set_max_free_chunks(size_t slot, size_t max_free_chunks) {
  if (slot < LUAIO_PMEMORY_SLOT_SIZE) {
    LuaIO_pmemory_slots[slot].max_free_chunks = max_free_chunks;
  }
}

void *LuaIO_pmemory__alloc(size_t size, size_t *capacity) {
  size_t real_size;

  if (size > LUAIO_PMEMORY_MAX_CHUNK_SIZE_LARGE) {
    real_size = size + sizeof(LuaIO_pool_chunk_t);
    LuaIO_pool_chunk_t *chunk = LuaIO_memalign(LUAIO_PMEMORY_ALIGNMENT, real_size);
    if (chunk == NULL) return NULL;

    if (capacity != NULL) {
      *capacity = real_size;
    }

    chunk->magic = LUAIO_PMEMORY_MAGIC;
    return (void*)((char*)chunk + sizeof(LuaIO_pool_chunk_t));
  }

  size_t slot, real_slot;
  if (size > LUAIO_PMEMORY_MAX_CHUNK_SIZE_MEDIUM) {
    slot = LuaIO_pmemory_slot_large(size);
    real_size = slot << LUAIO_PMEMORY_CHUNK_STEP_SHIFT_LARGE;
    real_slot = slot + LUAIO_PMEMORY_SLOT_OFFSET_LARGE;
  } else if (size > LUAIO_PMEMORY_MAX_CHUNK_SIZE_SMALL) {
    slot = LuaIO_pmemory_slot_medium(size);
    real_size = slot << LUAIO_PMEMORY_CHUNK_STEP_SHIFT_MEDIUM;
    real_slot = slot + LUAIO_PMEMORY_SLOT_OFFSET_MEDIUM;
  } else {
    slot = LuaIO_pmemory_slot_small(size);
    real_size = slot << LUAIO_PMEMORY_CHUNK_STEP_SHIFT_SMALL;
    real_slot = slot + LUAIO_PMEMORY_SLOT_OFFSET_SMALL;
  }

  void *p =  LuaIO_pool_alloc(&LuaIO_pmemory_slots[real_slot],
                              real_size,
                              LUAIO_PMEMORY_ALIGNMENT,
                              real_slot);
  if (p == NULL) return NULL;

  if (capacity != NULL) {
    *capacity = real_size;
  }

  return p; 
}

void LuaIO_pmemory__free(void *p) {
  if (p != NULL) {
    LuaIO_pool_chunk_t *chunk = (LuaIO_pool_chunk_t*)((char*)p - sizeof(LuaIO_pool_chunk_t));
    if (chunk->magic == LUAIO_PMEMORY_MAGIC) {
      LuaIO_free(chunk);
      return;
    }
   
    assert(chunk->magic != LUAIO_POOL_MAGIC);
    assert(chunk->magic < LUAIO_PMEMORY_SLOT_SIZE);

    LuaIO_pool_free(&LuaIO_pmemory_slots[chunk->magic], p);
  }
}
