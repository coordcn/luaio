/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "init.h"
#include "LuaIO.h"

static uint64_t LuaIO_start_time;
static lua_State *LuaIO_main_thread;

static void LuaIO_sleep_timeout(uv_timer_t *handle) {
  lua_State *L = handle->data;
  LuaIO_timer_free(handle);
  lua_pushinteger(L, 0);
  LuaIO_resume(L, 1);
}

static int LuaIO_sleep(lua_State *L) {
  lua_Integer delay = luaL_checkinteger(L, 1);
  uv_timer_t *timer = LuaIO_timer_alloc();
  if (timer == NULL) {
    lua_pushinteger(L, UV_ENOMEM);
    return 1;
  }

  timer->data = L;
  uv_timer_start(timer,
                 LuaIO_sleep_timeout,
                 delay,
                 0);

  return lua_yield(L, 0);
}

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
  LuaIO_date_init(); 
  LuaIO_pmemory_init(LUAIO_PMEMORY_MAX_FREE_CHUNKS);
  LuaIO_timer_init(LUAIO_TIMER_MAX_FREE_TIMERS);
  LuaIO_dns_init(L);
  LuaIO_tcp_connect_req_pool_init(LUAIO_TCP_CONNECT_REQ_POOL_MAX_FREE_CHUNKS);
  LuaIO_tcp_write_req_pool_init(LUAIO_TCP_WRITE_REQ_POOL_MAX_FREE_CHUNKS);
  LuaIO_tcp_shutdown_req_pool_init(LUAIO_TCP_SHUTDOWN_REQ_POOL_MAX_FREE_CHUNKS);
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
  
  /*date*/
  lua_pushcfunction(L, luaopen_date);
  lua_setfield(L, -2, "date");
  
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
  
  /*http_native*/
  lua_pushcfunction(L, luaopen_http);
  lua_setfield(L, -2, "http_native");

  /*fs_native*/
  lua_pushcfunction(L, luaopen_fs);
  lua_setfield(L, -2, "fs_native");
  
  lua_pop(L, 1);

  /*argv*/
  lua_createtable (L, argc, 0);
  for (int i = 0; i < argc; i++) {
    lua_pushstring (L, argv[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_setglobal(L, "argv");

  /*sleep(delay)*/
  lua_pushcfunction(L, LuaIO_sleep);
  lua_setglobal(L, "sleep");

  return 0;
}

lua_State *LuaIO_get_main_thread() {
  return LuaIO_main_thread;
}

uint64_t LuaIO_get_start_time() {
  return LuaIO_start_time;
}
