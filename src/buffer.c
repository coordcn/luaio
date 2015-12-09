/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "buffer.h"

#define LuaIO_buffer_check_buffer(L, name) \
  LuaIO_buffer_t* buffer = lua_touserdata(L, 1); \
  if (buffer == NULL || !LuaIO_is_buffer(buffer->type)) { \
    return luaL_argerror(L, 1, "buffer:"#name" error: buffer must be [userdata](read_buffer|write_buffer)\n"); \
  }

/* local capacity = buffer:capacity() */
int LuaIO_buffer_capacity(lua_State* L) {
  LuaIO_buffer_check_buffer(L, capacity());

  lua_pushinteger(L, buffer->capacity);
  return 1;
}

/* buffer:discard(n)  discard n bytes data
 * buffer:diacard()   discard rest data
 */
int LuaIO_buffer_discard(lua_State* L) {
  LuaIO_buffer_check_buffer(L, discard([n]));
  LuaIO_buffer_check_memory(L, diacard([n]));
  
  lua_Integer n = luaL_checkinteger(L, 2);

  char* start;
  if (n > 0) {
    char* pos = buffer->read_pos + n;
    if (pos < buffer->write_pos) {
      buffer->read_pos = pos;
    } else {
      start = buffer->start;
      buffer->read_pos = start;
      buffer->write_pos = start;
    }
  
    return 0;
  }

  if (n == -1) {
    start = buffer->start;
    buffer->read_pos = start;
    buffer->write_pos = start;

    return 0;
  }

  if (n < -1) {
    return luaL_argerror(L, 1, "buffer:discard([n]) error: n must be >= -1\n"); 
  }

  return 0;
}

int LuaIO_buffer_gc(lua_State* L) {
  LuaIO_buffer_check_buffer(L, __gc());

  char* start = buffer->start;
  if (start != NULL) {
    LuaIO_pmemory_free(start);
    buffer->capacity = 0;
    buffer->start = NULL;
  }

  return 0;
}
