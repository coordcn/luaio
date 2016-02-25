/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_CONFIG_H
#define LUAIO_CONFIG_H

#include "linux_config.h"

#ifndef offset_of
# define offset_of(type, member) ((intptr_t)((char*)(&(((type*)(0))->member))))
#endif

#ifndef container_of
# define container_of(ptr, type, member) ((type*)((char*)(ptr) - offset_of(type, member)))
#endif

#define LuaIO_align(size, alignment) (((size) + (alignment - 1)) & ~(alignment - 1))

#define LUAIO_BITS 64
#define LUAIO_POINTER_SIZE (sizeof(void*))

#define LUAIO_2_SHIFT             1
#define LUAIO_4_SHIFT             2
#define LUAIO_8_SHIFT             3
#define LUAIO_16_SHIFT            4
#define LUAIO_32_SHIFT            5
#define LUAIO_64_SHIFT            6
#define LUAIO_128_SHIFT           7
#define LUAIO_256_SHIFT           8
#define LUAIO_512_SHIFT           9
#define LUAIO_1K_SHIFT            10
#define LUAIO_2K_SHIFT            11
#define LUAIO_4K_SHIFT            12
#define LUAIO_8K_SHIFT            13
#define LUAIO_16K_SHIFT           14
#define LUAIO_32K_SHIFT           15
#define LUAIO_64K_SHIFT           16

#define LUAIO_HASH_ITEM_POOL_MAX_FREE_CHUNKS          1024
#define LUAIO_PMEMORY_MAX_FREE_CHUNKS                 1024
#define LUAIO_SIGNAL_MAX_FREE_CHUNKS                  16
#define LUAIO_TIMER_MAX_FREE_TIMERS                   1024
#define LUAIO_TCP_CONNECT_REQ_POOL_MAX_FREE_CHUNKS    1024
#define LUAIO_TCP_WRITE_REQ_POOL_MAX_FREE_CHUNKS      1024
#define LUAIO_TCP_SHUTDOWN_REQ_POOL_MAX_FREE_CHUNKS   256
#define LUAIO_FS_REQ_POOL_MAX_FREE_CHUNKS             1024

#endif /* LUAIO_CONFIG_H */
