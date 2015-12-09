/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_BUFFER_H
#define LUAIO_BUFFER_H

#include "pmemory.h"
#include "init.h"
#include "LuaIO.h"
#include <endian.h>

typedef struct {
  size_t type;
  size_t size;
  size_t capacity;
  char* start;
  char* read_pos;
  /*char* parse_pos;*/
  char* write_pos;
  char* end;
} LuaIO_buffer_t;

#define LuaIO_buffer_check_memory(L, name) \
  if (buffer->capacity == 0) { \
    return luaL_error(L, "buffer:"#name" error: no memory available\n"); \
  }

int LuaIO_buffer_capacity(lua_State* L);
int LuaIO_buffer_discard(lua_State* L);
int LuaIO_buffer_gc(lua_State* L);

#endif /* LUAIO_BUFFER_H */
