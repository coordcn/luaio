/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "init.h"
#include "LuaIO.h"

static uint64_t LuaIO_start_time;
static lua_State* LuaIO_main_thread;

static void LuaIO_platform_init() {
  /*ignore SGPIPE*/
  struct sigaction sa;
  LuaIO_memzero(&sa, sizeof(struct sigaction));
  sa.sa_handler = SIG_IGN;
  int ret = sigaction(SIGPIPE, &sa, NULL);
  assert(ret == 0);
}

int LuaIO_init(lua_State *L, int argc, char* argv[]) {
  /*config.h*/
  LuaIO_platform_init();
  LuaIO_pmemory_init(LUAIO_PMEMORY_MAX_FREE_CHUNKS);
  LuaIO_dns_init(L);
  LuaIO_fs_req_pool_init(LUAIO_FS_REQ_POOL_MAX_FREE_CHUNKS);

  LuaIO_start_time = uv_now(uv_default_loop());
  LuaIO_main_thread = L;

  /*preload*/
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "preload");
  lua_remove(L, -2);

  /*errno*/
  lua_pushcfunction(L, luaopen_errno);
  lua_setfield(L, -2, "errno");
 
  /*system*/
  lua_pushcfunction(L, luaopen_system);
  lua_setfield(L, -2, "system");
 
  /*process*/
  lua_pushcfunction(L, luaopen_process);
  lua_setfield(L, -2, "process");
  
  /*read_buffer*/
  lua_pushcfunction(L, luaopen_read_buffer);
  lua_setfield(L, -2, "read_buffer");
  
  /*write_buffer*/
  lua_pushcfunction(L, luaopen_write_buffer);
  lua_setfield(L, -2, "write_buffer");
  
  /*dns*/
  lua_pushcfunction(L, luaopen_dns);
  lua_setfield(L, -2, "dns");
  
  /*tcp_native*/
  lua_pushcfunction(L, luaopen_tcp);
  lua_setfield(L, -2, "tcp_native");
  
  /*fs_native*/
  lua_pushcfunction(L, luaopen_fs);
  lua_setfield(L, -2, "fs_native");
  
  /*http_parser*/
  /*lua_pushcfunction(L, luaopen_http_parser);*/
  /*lua_setfield(L, -2, "http_parser");*/

  lua_pop(L, 1);

  /*argv*/
  lua_createtable (L, argc, 0);
  for (int i = 0; i < argc; i++) {
    lua_pushstring (L, argv[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_setglobal(L, "argv");

  return 0;
}

lua_State* LuaIO_get_main_thread() {
  return LuaIO_main_thread;
}

uint64_t LuaIO_get_start_time() {
  return LuaIO_start_time;
}
