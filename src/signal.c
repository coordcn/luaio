/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "init.h"
#include "LuaIO.h"

typedef struct {
  lua_State   *current_thread;
  uv_signal_t handle;
  int         callback_ref;
} LuaIO_signal_t;

static char LuaIO_signal_metatable_key;

#define LuaIO_signal_check_signal(L, name) \
  uv_signal_t *signal = lua_touserdata(L, 1); \
  if (socket == NULL) { \
    return luaL_argerror(L, 1, "signal:"#name" error: signal must be [userdata](signal)\n"); \
  }

/*local signal = signal.new()*/
static int LuaIO_signal_new(lua_State *L) {
  LuaIO_signal_t *signal = lua_newuserdata(L, sizeof(LuaIO_signal_t));
  if (signal == NULL) {
    lua_pushnil(L);
    return 1;
  }

  uv_loop_t *loop = uv_default_loop();
  uv_signal_init *handle = &signal->handle;
  uv_signal_init(loop, handle);
  uv_unref((uv_handle_t*)handle);
  signal->current_thread = L;
  signal->callback_ref = LUA_NOREF;

  lua_pushlightuserdata(L, &LuaIO_signal_metatable_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_setmetatable(L, -2);
  return 1;
}

int luaopen_signal(lua_State *L) {
  /*signal metatable*/
  lua_pushlightuserdata(L, &LuaIO_signal_metatable_key);

  lua_createtable(L, 0, 16);
  LuaIO_function(LuaIO_signal_start, "start")
  LuaIO_function(LuaIO_signal_stop, "stop")
  LuaIO_function(LuaIO_signal_close, "close")

  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  /*save tcp metatable*/
  lua_rawset(L, LUA_REGISTRYINDEX);

  luaL_Reg lib[] = {
    { "new", LuaIO_signal_new },
    { "__newindex", LuaIO_cannot_change },
    { NULL, NULL }
  };

  lua_createtable(L, 0, 0);

  luaL_newlib(L, lib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected.");
  lua_setfield(L, -2, "__metatable");

  lua_setmetatable(L, -2);

  return 1;
}
