/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_STRING_H
#define LUAIO_STRING_H

#include "luaio_alloc.h"
#include "luaio_pmemory.h"

#define luaio_lower(c)              (unsigned char) (c | 0x20)
#define luaio_upper(c)              (unsigned char) (c & ~0x20)

#define luaio_strlen                strlen
#define luaio_strncmp               strncmp
#define luaio_strncasecmp           strncasecmp

static inline char *luaio_strndup(const char *src, size_t n) {
  char *dst = luaio_palloc(n + 1);
  if (dst == NULL) return NULL;

  dst[n] = '\0';
  luaio_memcpy(dst, src, n);

  return dst;
}

#if LUAIO_BITS == 32

#define LUAIO_SHORT_STRING_LENGTH   32

#define luaio_streq                 luaio_streq_32
#define luaio_strcaseeq             luaio_strcaseeq_32

#elif LUAIO_BITS == 64

#define LUAIO_SHORT_STRING_LENGTH   64

#define luaio_streq                 luaio_streq_64
#define luaio_strcaseeq             luaio_strcaseeq_64

#else
#error LUAIO_BITS not 64 or 32
#endif

int luaio_streq_32(const char *s1, const char *s2, size_t n);
int luaio_strcaseeq_32(const char *s1, const char *s2, size_t n);
int luaio_streq_64(const char *s1, const char *s2, size_t n);
int luaio_strcaseeq_64(const char *s1, const char *s2, size_t n);

#endif /* LUAIO_STRING_H */
