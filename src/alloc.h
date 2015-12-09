/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @overview: alloc, calloc, realloc, free, memalign from nginx
 * @reference: ngx_alloc.h
 */

#ifndef LUAIO_ALLOC_H
#define LUAIO_ALLOC_H

#include "config.h"

void* LuaIO_malloc(size_t size);
void* LuaIO_calloc(size_t size);
void* LuaIO_realloc(void* p, size_t size);
#define LuaIO_free                  free

/* Linux has memalign() or posix_memalign()
 * Solaris has memalign()
 * FreeBSD 7.0 has posix_memalign(), besides, early version's malloc()
 * aligns allocations bigger than page size at the page boundary
 */

#define HAVE_POSIX_MEMALIGN 1
/*#define HAVE_MEMALIGN 1*/

#if (HAVE_POSIX_MEMALIGN || HAVE_MEMALIGN)

void* LuaIO_memalign(size_t align, size_t size);

#else

#define LuaIO_memalign(align, size) LuaIO_malloc(size)

#endif

#define LuaIO_lower(c)              (unsigned char) (c | 0x20)
#define LuaIO_upper(c)              (unsigned char) (c & ~0x20)

#define LuaIO_memset                memset
#define LuaIO_memcpy                memcpy
#define LuaIO_memmove               memmove
#define LuaIO_memcmp                memcmp
#define LuaIO_memzero(p, size)      LuaIO_memset(p, 0, size)

#define LuaIO_strlen                strlen
#define LuaIO_strncmp               strncmp
#define LuaIO_strncasecmp           strncasecmp

#define LuaIO_strcmp1(s, c0) \
  s[0] == c0
  
#define LuaIO_strcmp2(s, c0, c1) \
  (s[0] == c0 && s[1] == c1)
  
#define LuaIO_strcmp3(s, c0, c1, c2) \
  (s[0] == c0 && s[1] == c1 && s[2] == c2)
  
#define LuaIO_strcmp4(s, c0, c1, c2, c3) \
  (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3)
  
#define LuaIO_strcmp5(s, c0, c1, c2, c3, c4) \
  (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4)
  
#define LuaIO_strcmp6(s, c0, c1, c2, c3, c4, c5) \
  (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && s[5] == c5)
  
#define LuaIO_strcmp7(s, c0, c1, c2, c3, c4, c5, c6) \
  (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && s[5] == c5 && s[6] == c6)
  
#define LuaIO_strcmp8(s, c0, c1, c2, c3, c4, c5, c6, c7) \
  (s[0] == c0 && s[1] == c1 && s[2] == c2 && s[3] == c3 && s[4] == c4 && s[5] == c5 && s[6] == c6 && s[7] == c7)
  
static inline char* LuaIO_strndup(const char* src, size_t n) {
  char* dst = LuaIO_malloc(n + 1);
  if (dst == NULL) return NULL;

  dst[n] = '\0';
  LuaIO_memcpy(dst, src, n);

  return dst;
}

#endif /*LUAIO_ALLOC_H*/
