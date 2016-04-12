/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_BUFFER_H
#define LUAIO_BUFFER_H

#include "luaio_init.h"

typedef struct {
  size_t    type;
  size_t    size;
  size_t    capacity;
  char      *start;
  char      *read_pos;
  char      *write_pos;
  char      *end;
} luaio_buffer_t;

#define luaio_buffer_check_memory(L, name) \
  if (buffer->capacity == 0) { \
    return luaL_error(L, "buffer:"#name" error: no memory available\n"); \
  }

int luaio_buffer_capacity(lua_State *L);
int luaio_buffer_discard(lua_State *L);
int luaio_buffer_gc(lua_State *L);

#endif /* LUAIO_BUFFER_H */
