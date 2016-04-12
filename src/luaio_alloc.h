/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @overview: alloc, calloc, realloc, free, memalign from nginx
 * @reference: ngx_alloc.h
 */

#ifndef LUAIO_ALLOC_H
#define LUAIO_ALLOC_H

#include "luaio_config.h"

void *luaio_malloc(size_t size);
void *luaio_realloc(void *p, size_t size);
#define luaio_free free

/* Linux has memalign() or posix_memalign()
 * Solaris has memalign()
 * FreeBSD 7.0 has posix_memalign(), besides, early version's malloc()
 * aligns allocations bigger than page size at the page boundary
 */
#define HAVE_POSIX_MEMALIGN 1
/*#define HAVE_MEMALIGN 1*/

#if (HAVE_POSIX_MEMALIGN || HAVE_MEMALIGN)

void* luaio_memalign(size_t align, size_t size);

#else

#define luaio_memalign(align, size) luaio_malloc(size)

#endif

#define luaio_memset                memset
#define luaio_memcpy                memcpy
#define luaio_memmove               memmove
#define luaio_memcmp                memcmp
#define luaio_memchr                memchr
#define luaio_memzero(p, size)      memset(p, 0, size)

#endif /*LUAIO_ALLOC_H*/
