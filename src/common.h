/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_COMMON_H
#define LUAIO_COMMON_H

#include "LuaIO.h"

int LuaIO_parse_socket_address(lua_State* L, struct sockaddr_storage* addr);

#define LuaIO_check_data(L, index, name) \
  uv_buf_t* bufs; \
  uv_buf_t* tmp = NULL; \
  uv_buf_t buf; \
  size_t count; \
  size_t len; \
  char* read_pos; \
  size_t bytes = 0; \
  int type = lua_type(L, index); \
  if (type == LUA_TSTRING) { \
    buf.base = (char*) lua_tolstring(L, index, &len); \
    buf.len = len; \
    bytes += len; \
    count = 1; \
    bufs = &buf; \
  } else if (type == LUA_TUSERDATA) { \
    LuaIO_buffer_t* buffer = lua_touserdata(L, index); \
    if (!LuaIO_is_buffer(buffer->type)) { \
      return luaL_argerror(L, index, #name" error: data is userdata, but not Buffer\n"); \
    } \
    \
    if (buffer->capacity == 0) { \
      return luaL_argerror(L, index, #name" error: data is Buffer, but no memory available\n"); \
    } \
    \
    read_pos = buffer->read_pos; \
    len = buffer->write_pos - read_pos; \
    buf.base = read_pos; \
    buf.len = len; \
    bytes += len; \
    count = 1; \
    bufs = &buf; \
  } else if (type == LUA_TTABLE) { \
    count = lua_rawlen(L, index); \
    bufs = LuaIO_pmemory_alloc(sizeof(uv_buf_t) * count); \
    if (bufs == NULL) { \
      lua_pushinteger(L, 0); \
      lua_pushinteger(L, UV_ENOMEM); \
      return 2; \
    } \
    \
    tmp = bufs; \
    for (size_t i = 0; i < count; ++i) { \
      size_t pos = i + 1; \
      lua_rawgeti(L, index, pos); \
      \
      int ttype = lua_type(L, -1); \
      if (ttype == LUA_TSTRING) { \
        bufs[i].base = (char*) lua_tolstring(L, -1, &len); \
        bufs[i].len = len; \
        bytes += len; \
      } else if (ttype == LUA_TUSERDATA) { \
        LuaIO_buffer_t* buffer = lua_touserdata(L, -1); \
        if (!LuaIO_is_buffer(buffer->type)) { \
          return luaL_error(L, #name" error: data[%d] is userdata, but not Buffer\n", pos); \
        } \
        \
        if (buffer->capacity == 0) { \
          return luaL_error(L, #name" error: data[%d] is Buffer, but no memory available\n", pos); \
        } \
        \
        read_pos = buffer->read_pos; \
        len = buffer->write_pos - read_pos; \
        buf.base = read_pos; \
        buf.len = len; \
        bytes += len; \
      } else { \
        return luaL_error(L, #name" error: data[%d] must be string or Buffer\n", pos); \
      } \
      \
      lua_pop(L, 1); \
    } \
  } else { \
    return luaL_argerror(L, index, #name" error: data must be [string|Buffer|table(string|Buffer)]\n"); \
  }

#endif /* LUAIO_COMMON_H */
