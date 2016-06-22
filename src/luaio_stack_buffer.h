/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_STACK_BUFFER_H
#define LUAIO_STACK_BUFFER_H

#include "luaio_pmemory.h"

#define LUAIO_STACK_BUFFER_SIZE 8192
typedef struct {
  char    *base;
  size_t  capacity;
  size_t  size;
  char    buf[LUAIO_STACK_BUFFER_SIZE];
} luaio_stack_buffer_t;

static inline void *luaio_stack_buffer_init(luaio_stack_buffer_t *b, size_t size) {
  b->size = 0;

  if (size > LUAIO_STACK_BUFFER_SIZE) {
    char *p = luaio_palloc(size);
    if (p == NULL) {
      b->base = b->buf;
      b->capacity = 0;
      return NULL;
    }

    b->base = p;
    b->capacity = luaio_pmemory_get_capacity(p);
  } else {
    b->base = b->buf;
    b->capacity = LUAIO_STACK_BUFFER_SIZE;
  }

  return (void*)b->base;
}

static inline void luaio_stack_buffer_free(luaio_stack_buffer_t *b) {
  if (b->base != b->buf) luaio_pfree(b->base);
}

#endif /* LUAIO_STACK_BUFFER_H */
