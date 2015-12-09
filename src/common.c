/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "common.h"

int LuaIO_cannot_change(lua_State* L) {
  return luaL_error(L, "table fields cannot be changed.");
}

int LuaIO_parse_socket_address(lua_State* L, struct sockaddr_storage* address) {
  char ip[INET6_ADDRSTRLEN];
  struct sockaddr_in* addr_in;
  struct sockaddr_in6* addr_in6;
  int family = 0;
  int port = 0;

  switch (address->ss_family) {
    case AF_INET:
      addr_in = (struct sockaddr_in*)address;
      uv_inet_ntop(AF_INET, &(addr_in->sin_addr), ip, sizeof(ip));
      family = 4;
      port = ntohs(addr_in->sin_port);
      break;
    case AF_INET6:
      addr_in6 = (struct sockaddr_in6*)address;
      uv_inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip, sizeof(ip));
      family = 6;
      port = ntohs(addr_in6->sin6_port);
      break;
    default:
      lua_pushnil(L);
      return UV_EAI_ADDRFAMILY;
  }

  lua_createtable(L, 0, 3);
  lua_pushinteger(L, family);
  lua_setfield(L, -2, "family");
  lua_pushinteger(L, port);
  lua_setfield(L, -2, "port");
  lua_pushstring(L, ip);
  lua_setfield(L, -2, "address");
  return 0;
}
