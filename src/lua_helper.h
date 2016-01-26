/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_LUA_HELPER_H
#define LUAIO_LUA_HELPER_H

#include "lua.h"
#include "lauxlib.h"

static inline int LuaIO_get_integer(lua_State *L, 
                                    int index, 
                                    const char *name, 
                                    lua_Integer *value) {
  int ret = 1;

  lua_pushstring(L, name);
  int type = lua_rawget(L, index);
  if (type == LUA_TNIL) {
    ret = 0;
  } else if (lua_isinteger(L, -1)) {
    *value = lua_tointeger(L, -1);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;
}

static inline int LuaIO_check_integer(lua_State *L, 
                                      int index, 
                                      const char *name, 
                                      lua_Integer *value) {
  int ret = 1;

  lua_pushstring(L, name);
  lua_rawget(L, index);
  if (lua_isinteger(L, -1)) {
    *value = lua_tointeger(L, -1);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;
}

static inline int LuaIO_get_number(lua_State *L, 
                                   int index, 
                                   const char *name, 
                                   lua_Number *value) {
  int ret = 1;

  lua_pushstring(L, name);
  int type = lua_rawget(L, index);
  if (type == LUA_TNIL) {
    ret = 0;
  } else if (type == LUA_TNUMBER) {
    *value = lua_tonumber(L, -1);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;
}

static inline int LuaIO_check_number(lua_State *L, 
                                     int index, 
                                     const char *name, 
                                     lua_Number *value) {
  int ret = 1;

  lua_pushstring(L, name);
  int type = lua_rawget(L, index);
  if (type == LUA_TNUMBER) {
    *value = lua_tonumber(L, -1);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;

}

static inline int LuaIO_get_string(lua_State *L, 
                                   int index, 
                                   const char *name, 
                                   const char *value __attribute__((unused))) {
  int ret = 1;

  lua_pushstring(L, name);
  int type = lua_rawget(L, index);
  if (type == LUA_TNIL) {
    ret = 0;
  } else if (type == LUA_TSTRING) {
    value = lua_tostring(L, -1);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;
}

static inline int LuaIO_check_string(lua_State *L, 
                                     int index, 
                                     const char *name, 
                                     const char *value __attribute__((unused))) {
  int ret = 1;

  lua_pushstring(L, name);
  int type = lua_rawget(L, index);
  if (type == LUA_TSTRING) {
    value = lua_tostring(L, -1);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;
}

static inline int LuaIO_get_lstring(lua_State *L, 
                                    int index, 
                                    const char *name, 
                                    const char *value __attribute__((unused)), size_t* n) {
  int ret = 1;

  lua_pushstring(L, name);
  int type = lua_rawget(L, index);
  if (type == LUA_TNIL) {
    ret = 0;
  } else if (type == LUA_TSTRING) {
    value = lua_tolstring(L, -1, n);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;
}

static inline int LuaIO_check_lstring(lua_State *L, 
                                      int index, 
                                      const char *name, 
                                      const char *value __attribute__((unused)), 
                                      size_t* n) {
  int ret = 1;

  lua_pushstring(L, name);
  int type = lua_rawget(L, index);
  if (type == LUA_TSTRING) {
    value = lua_tolstring(L, -1, n);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;
}

static inline int LuaIO_get_boolean(lua_State *L, 
                                    int index, 
                                    const char *name, 
                                    int *value) {
  int ret = 1;

  lua_pushstring(L, name);
  int type = lua_rawget(L, index);
  if (type == LUA_TNIL) {
    ret = 0;
  } else if (type == LUA_TBOOLEAN) {
    *value = lua_toboolean(L, -1);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;
}

static inline int LuaIO_check_boolean(lua_State *L, 
                                      int index, 
                                      const char *name, 
                                      int *value) {
  int ret = 1;

  lua_pushstring(L, name);
  int type = lua_rawget(L, index);
  if (type == LUA_TBOOLEAN) {
    *value = lua_toboolean(L, -1);
    ret = 0;
  }
  lua_pop(L, 1);

  return ret;
}

#endif /* LUAIO_LUA_HELPER_H */
