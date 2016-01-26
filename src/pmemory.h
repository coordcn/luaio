/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: allocate [256, 128k] bytes memory by memory pool
 *            < 256 => 256
 *            > 128K => malloc
 *            [256, 512, 768, 1024] step = 256
 *            [2048, 4096, 6144, 8192] step = 2048
 *            [16, 32, 48, 64, 80, 96, 112, 128] step = 16k
 */

#ifndef LUAIO_PMEMORY_H
#define LUAIO_PMEMORY_H

#include "alloc.h"
#include "palloc.h"

#define LUAIO_PMEMORY_ALIGNMENT                     sizeof(unsigned long)
#define LUAIO_PMEMORY_SLOT_SIZE                     16

#define LUAIO_PMEMORY_CHUNK_STEP_SMALL              256
#define LUAIO_PMEMORY_CHUNK_STEP_SHIFT_SMALL        LUAIO_256_SHIFT

#define LUAIO_PMEMORY_CHUNK_STEP_MEDIUM             2048
#define LUAIO_PMEMORY_CHUNK_STEP_SHIFT_MEDIUM       LUAIO_2K_SHIFT

#define LUAIO_PMEMORY_CHUNK_STEP_LARGE              (16*1024)
#define LUAIO_PMEMORY_CHUNK_STEP_SHIFT_LARGE        LUAIO_16K_SHIFT

#define LUAIO_PMEMORY_MAX_CHUNK_SIZE_SMALL          1024
#define LUAIO_PMEMORY_MAX_CHUNK_SIZE_MEDIUM         8192
#define LUAIO_PMEMORY_MAX_CHUNK_SIZE_LARGE          (128*1024)

#define LUAIO_PMEMORY_SLOT_OFFSET_SMALL              (-1)
#define LUAIO_PMEMORY_SLOT_OFFSET_MEDIUM             (3)
#define LUAIO_PMEMORY_SLOT_OFFSET_LARGE              (7)

#define LuaIO_pmemory_slot_small(size) \
  (LuaIO_align(LUAIO_PMEMORY_CHUNK_STEP_SMALL, (size)) >> LUAIO_PMEMORY_CHUNK_STEP_SHIFT_SMALL)

#define LuaIO_pmemory_slot_medium(size) \
  (LuaIO_align(LUAIO_PMEMORY_CHUNK_STEP_MEDIUM, (size)) >> LUAIO_PMEMORY_CHUNK_STEP_SHIFT_MEDIUM)

#define LuaIO_pmemory_slot_large(size) \
  (LuaIO_align(LUAIO_PMEMORY_CHUNK_STEP_LARGE, (size)) >> LUAIO_PMEMORY_CHUNK_STEP_SHIFT_LARGE)

#define LuaIO_pmemory_alloc(size) LuaIO_pmemory__alloc(size, NULL)
#define LuaIO_pmemory_free(p) LuaIO_pmemory__free(p)

void LuaIO_pmemory_init(size_t max_free_chunks);
void LuaIO_pmemory_set_max_free_chunks(size_t slot, size_t max_free_chunks);

void *LuaIO_pmemory__alloc(size_t size, size_t *capacity);
void LuaIO_pmemory__free(void *p);

#endif /* LUAIO_PMEMORY_H */
