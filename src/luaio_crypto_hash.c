/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio.h"
#include "luaio_init.h"
#include "luaio_crypto.h"

static char luaio_crypto_hash_metatable_key;

static int luaio_crypto_hash_new(lua_State *L) {
}

static int luaio_crypto_hash_update(lua_State *L) {
}

static int luaio_crypto_hash_final(lua_State *L) {
}

static int luaio_crypto_hash_reset(lua_State *L) {
}

static int luaio_crypto_hash_gc(lua_State *L) {
}

static int luaio_crypto_hash(lua_State *L) {
}

int luaopen_crypto_hash(lua_State *L) {
  /*hash context metatable*/
  luaL_Reg crypto_hash_mtlib[] = {
    { "update", luaio_crypto_hash_update },
    { "final", luaio_crypto_hash_final },
    { "reset", luaio_crypto_hash_reset },
    { "__gc", luaio_crypto_hash_gc },
    { NULL, NULL }
  };

  lua_pushlightuserdata(L, &luaio_crypto_hash_metatable_key);
  luaL_newlib(L, crypto_hash_mtlib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_rawset(L, LUA_REGISTRYINDEX);

  luaL_Reg lib[] = {
    { "new", luaio_crypto_hash_new },
    { "__call", luaio_crypto_hash },
    { "__newindex", luaio_cannot_change },
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
