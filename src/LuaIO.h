/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview:
 */

#ifndef LUAIO_COORD_H
#define LUAIO_COORD_H

#include "lua.h"
#include "lauxlib.h"
#include "uv.h"
#include "ares.h"

#include "config.h"

#include "list.h"
#include "alloc.h"
#include "palloc.h"
#include "pmemory.h"
#include "hash.h"
#include "buffer.h"

#define CR '\r'
#define LF '\n'
#define CRLF "\r\n"

/*errno*/
#define LUAIO_ARES_MAGIC                    -16834
#define LUAIO_EAGAIN                        -9527
#define LUAIO_EXCEED_BUFFER_CAPACITY        -9528

#define LUAIO_ERRNO_MAP(XX)                                             \
  XX(EAGAIN, "try again")                                               \
  XX(EXCEED_BUFFER_CAPACITY, "exceed buffer capacity")                  \

#define LuaIO_set_bit(value, shift) (value |= (1 << shift))
#define LuaIO_clear_bit(value, shift) (value &= ~(1 << shift))
#define LuaIO_check_bit(value, shift) (value & (1 << shift))

#define LUAIO_TYPE_SOCKET             1
#define LUAIO_TYPE_READ_BUFFER        4
#define LUAIO_TYPE_WRITE_BUFFER       6 

#define LuaIO_is_buffer(type) LuaIO_check_bit(type, 2)

/*init.c*/
lua_State* LuaIO_get_main_thread();

/*LuaIO.c*/
int LuaIO_cannot_change(lua_State* L);

static inline void LuaIO_resume(lua_State* L, int nargs) {
  int ret = lua_resume(L, NULL, nargs);
  if (ret > LUA_YIELD) {
    const char* error_string = lua_tostring(L, -1);
    luaL_error(L, "lua_resume() error: %s\n", error_string ? error_string : "unknow");
  }
}

static inline void LuaIO_pcall(lua_State* L, int nargs) {
  int ret = lua_pcall(L, nargs, 0, 0);
  if (ret != LUA_OK) {
    const char* error_string = lua_tostring(L, -1);
    luaL_error(L, "lua_pcall() error: %s\n", error_string ? error_string : "unknow");
  }
}

#endif /* LUAIO_COORD_H */
