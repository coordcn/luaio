/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "LuaIO.h"

#define ARES_ERRNO_MAP(XX)                                                  \
  XX(ENODATA, "DNS server returned answer with no data")                    \
  XX(EFORMERR, "DNS server claims query was misformatted")                  \
  XX(ESERVFAIL, "DNS server returned general failure")                      \
  XX(ENOTFOUND, "Domain name not found")                                    \
  XX(ENOTIMP, "DNS server does not implement requested operation")          \
  XX(EREFUSED, "DNS server refused query")                                  \
  XX(EBADQUERY, "misformatted DNS query")                                   \
  XX(EBADNAME, "misformatted domain name")                                  \
  XX(EBADFAMILY, "unsupported address family")                              \
  XX(EBADRESP, "misformatted DNS reply")                                    \
  XX(ECONNREFUSED, "could not contact DNS servers")                         \
  XX(ETIMEOUT, "timeout while contacting DNS servers")                      \
  XX(EOF, "end of file")                                                    \
  XX(EFILE, "error reading file")                                           \
  XX(ENOMEM, "out of memory")                                               \
  XX(EDESTRUCTION, "channel is being destroyed")                            \
  XX(EBADSTR, "misformatted string")                                        \
  XX(EBADFLAGS, "illegal flags specified")                                  \
  XX(ENONAME, "given hostname is not numeric")                              \
  XX(EBADHINTS, "illegal hints flags specified")                            \
  XX(ENOTINITIALIZED, "c-ares library initialization not yet performed")    \
  XX(ELOADIPHLPAPI, "error loading iphlpapi.dll")                           \
  XX(EADDRGETNETWORKPARAMS, "could not find GetNetworkParams function")     \
  XX(ECANCELLED, "DNS query cancelled")                                      \

#define UV_ERRSTR_GEN(name, str) \
  case UV_##name: \
    msg = str; \
    break;

#define ARES_ERRSTR_GEN(name, str) \
  case ARES_##name: \
    msg = str; \
    break;

#define LUAIO_ERRSTR_GEN(name, str) \
  case LUAIO_##name: \
    msg = str; \
    break;

static int LuaIO_errno_parse(lua_State* L) {
  int err = luaL_checkinteger(L, 1);
  const char* msg;

  switch (err) {
    case 0:
      msg = "success";
      break;
    
    UV_ERRNO_MAP(UV_ERRSTR_GEN)

    ARES_ERRNO_MAP(ARES_ERRSTR_GEN)

    LUAIO_ERRNO_MAP(LUAIO_ERRSTR_GEN)

    default:
      msg = "unknown error";
  }

  lua_pushstring(L, msg);
  return 1;
}

#undef LUAIO_ERRSTR_GEN

#undef ARES_ERRSTR_GEN

#undef UV_ERRSTR_GEN

static void LuaIO_errno_setup_constants(lua_State* L) {
#define UV_ERRNO_GEN(name, _) \
  lua_pushinteger(L, UV_##name); \
  lua_setfield(L, -2, "UV_"#name);

  UV_ERRNO_MAP(UV_ERRNO_GEN)
#undef UV_ERRNO_GEN

#define ARES_ERRNO_GEN(name, _) \
  lua_pushinteger(L, ARES_##name); \
  lua_setfield(L, -2, "ARES_"#name);

  ARES_ERRNO_MAP(ARES_ERRNO_GEN)
#undef ARES_ERRNO_GEN

#define LUAIO_ERRNO_GEN(name, _) \
  lua_pushinteger(L, LUAIO_##name); \
  lua_setfield(L, -2, "LUAIO_"#name);

  LUAIO_ERRNO_MAP(LUAIO_ERRNO_GEN)
#undef LUAIO_ERRNO_GEN
}

int luaopen_errno(lua_State *L) {
  luaL_Reg lib[] = {
    { "parse", LuaIO_errno_parse },
    { "__newindex", LuaIO_cannot_change },
    { NULL, NULL}
  };

  lua_createtable(L, 0, 0);

  luaL_newlib(L, lib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected.");
  lua_setfield(L, -2, "__metatable");
  LuaIO_errno_setup_constants(L);

  lua_setmetatable(L, -2);

  return 1;
}
