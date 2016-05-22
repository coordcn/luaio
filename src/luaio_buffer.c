/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio.h"
#include "luaio_buffer.h"

#define luaio_buffer_check_buffer(L, name) \
  luaio_buffer_t *buffer = lua_touserdata(L, 1); \
  if (buffer == NULL || !luaio_is_buffer(buffer->type)) { \
    return luaL_argerror(L, 1, "buffer:"#name" error: buffer must be [userdata](buffer)\n"); \
  }

/* local capacity = buffer:capacity() */
int luaio_buffer_capacity(lua_State *L) {
  luaio_buffer_check_buffer(L, capacity());
  lua_pushinteger(L, buffer->capacity);
  return 1;
}

/* buffer:discard(n)  discard n bytes data
 * buffer:diacard()   discard rest data
 */
int luaio_buffer_discard(lua_State *L) {
  luaio_buffer_check_buffer(L, discard([n]));
  luaio_buffer_check_memory(L, diacard([n]));
  lua_Integer n = luaL_checkinteger(L, 2);

  /*do nothing*/
  if (n == 0) {
    lua_pushinteger(L, n);
    return 1;
  }

  char *read_pos = buffer->read_pos;
  int rest_size = buffer->write_pos - read_pos;
  if (n > 0 && n < rest_size) {
    buffer->read_pos = read_pos + n;
    lua_pushinteger(L, n);
    return 1;
  }

  char *start = buffer->start;
  buffer->read_pos = start;
  buffer->write_pos = start;

  lua_pushinteger(L, rest_size);
  return 1;
}

int luaio_buffer_gc(lua_State *L) {
  luaio_buffer_check_buffer(L, __gc());

  char *start = buffer->start;
  if (start != NULL) {
    luaio_pfree(start);
    buffer->capacity = 0;
    buffer->start = NULL;
  }

  return 0;
}
