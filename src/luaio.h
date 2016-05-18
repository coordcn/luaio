/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview:
 */

#ifndef LUAIO_COORD_H
#define LUAIO_COORD_H

#include "uv.h"
#include "lua.h"
#include "lauxlib.h"

#include "luaio_config.h"

#include "luaio_list.h"
#include "luaio_pmemory.h"
#include "luaio_string.h"
#include "luaio_hash.h"
#include "luaio_buffer.h"

#if defined(LUAIO_WINDOWS)

#define LUAIO_COLOR_RESET
#define LUAIO_COLOR_BLACK
#define LUAIO_COLOR_RED
#define LUAIO_COLOR_GREEN
#define LUAIO_COLOR_YELLOW
#define LUAIO_COLOR_BLUE
#define LUAIO_COLOR_MAGENTA
#define LUAIO_COLOR_CYAN
#define LUAIO_COLOR_WHITE

#else

#define LUAIO_COLOR_RESET   "\033[0m"
#define LUAIO_COLOR_BLACK   "\033[0;30m"
#define LUAIO_COLOR_RED     "\033[0;31m"
#define LUAIO_COLOR_GREEN   "\033[0;32m"
#define LUAIO_COLOR_YELLOW  "\033[0;33m"
#define LUAIO_COLOR_BLUE    "\033[0;34m"
#define LUAIO_COLOR_MAGENTA "\033[0;35m"
#define LUAIO_COLOR_CYAN    "\033[0;36m"
#define LUAIO_COLOR_WHITE   "\033[0;37m"

#endif

#define LUAIO_COLOR_ERROR   LUAIO_COLOR_GREEN

/*errno*/
#define LUAIO_ARES_MAGIC                    -16834
#define LUAIO_EAGAIN                        -9527
#define LUAIO_EXCEED_BUFFER_CAPACITY        -9528
#define LUAIO_ENOTFILE                      -9529
#define LUAIO_EBAD_UTF8_CHAR                -9530
#define LUAIO_EINCOMPLETE_UTF8_CHAR         -9531

#define LUAIO_ERRNO_MAP(XX)                                             \
  XX(EAGAIN, "try again")                                               \
  XX(EXCEED_BUFFER_CAPACITY, "exceed buffer capacity")                  \
  XX(ENOTFILE, "not a file")                                            \
  XX(EBAD_UTF8_CHAR, "bad utf8 char")                                   \
  XX(EINCOMPLETE_UTF8_CHAR, "incomplete utf8 char")                     \

#define luaio_set_bit(value, shift)         (value |= (1 << shift))
#define luaio_clear_bit(value, shift)       (value &= ~(1 << shift))
#define luaio_check_bit(value, shift)       (value & (1 << shift))

#define LUAIO_TYPE_SOCKET                   1
#define LUAIO_TYPE_READ_BUFFER              4
#define LUAIO_TYPE_WRITE_BUFFER             6 

#define luaio_is_buffer(type) luaio_check_bit(type, 2)

#define luaio_constant(name) \
  lua_pushinteger(L, name); \
  lua_setfield(L, -2, #name); \

#define luaio_function(fun, name) \
  lua_pushcfunction(L, fun); \
  lua_setfield(L, -2, name); \

#define luaio_setinteger(name, value) \
  lua_pushinteger(L, value); \
  lua_setfield(L, -2, name); \

#define luaio_setstring(name, str) \
  lua_pushstring(L, str); \
  lua_setfield(L, -2, name); \

#define luaio_setlstring(name, str, len) \
  lua_pushlstring(L, str, len); \
  lua_setfield(L, -2, name); \

/*luaio_util.c*/
int luaio_cannot_change(lua_State *L);

static inline void luaio_resume(lua_State *L, int nargs) {
  int ret = lua_resume(L, NULL, nargs);
  if (ret > LUA_YIELD) {
    const char *error_string = lua_tostring(L, -1);
    luaL_error(L, "luaio_resume() error: \n%s\n", error_string ? error_string : "unknow");
  }
}

static inline void luaio_pcall(lua_State *L, int nargs) {
  int ret = lua_pcall(L, nargs, 0, 0);
  if (ret != LUA_OK) {
    const char *error_string = lua_tostring(L, -1);
    luaL_error(L, "luaio_pcall() error:\n%s\n", error_string ? error_string : "unknow");
  }
}

#endif /* LUAIO_COORD_H */
