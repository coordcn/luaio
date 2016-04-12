/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio.h"
#include "luaio_init.h"
#include "luaio_timer.h"

static uint64_t luaio_start_time;
static lua_State *luaio_main_thread;

static void luaio_sleep_timeout(uv_timer_t *handle) {
  lua_State *L = handle->data;
  luaio_timer_free(handle);
  lua_pushinteger(L, 0);
  luaio_resume(L, 1);
}

static int luaio_sleep(lua_State *L) {
  lua_Integer delay = luaL_checkinteger(L, 1);
  uv_timer_t *timer = luaio_timer_alloc();
  if (timer == NULL) {
    lua_pushinteger(L, UV_ENOMEM);
    return 1;
  }

  timer->data = L;
  uv_timer_start(timer,
                 luaio_sleep_timeout,
                 delay,
                 0);

  return lua_yield(L, 0);
}

static void luaio_platform_init() {
  /*ignore SGPIPE*/
  struct sigaction sa;
  luaio_memzero(&sa, sizeof(struct sigaction));
  sa.sa_handler = SIG_IGN;
  int ret = sigaction(SIGPIPE, &sa, NULL);
  assert(ret == 0);
}

int luaio_init(lua_State *L, int argc, char* argv[]) {
  /*config.h*/
  luaio_platform_init();
  luaio_timer_init(LUAIO_MAX_FREE_TIMERS);
  luaio_date_init(); 
  luaio_dns_init(L);

  luaio_start_time = uv_now(uv_default_loop());
  luaio_main_thread = L;

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
 
  /*signal*/
  lua_pushcfunction(L, luaopen_signal);
  lua_setfield(L, -2, "signal");
  
  /*process_native*/
  lua_pushcfunction(L, luaopen_process);
  lua_setfield(L, -2, "process_native");
  
  /*strlib_native*/
  lua_pushcfunction(L, luaopen_strlib);
  lua_setfield(L, -2, "strlib_native");
  
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

  /*__ARGV__*/
  lua_createtable (L, argc, 0);
  for (int i = 0; i < argc; i++) {
    lua_pushstring (L, argv[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_setglobal(L, "__ARGV__");

  /*sleep(delay)*/
  lua_pushcfunction(L, luaio_sleep);
  lua_setglobal(L, "sleep");

  return 0;
}

lua_State *luaio_get_main_thread() {
  return luaio_main_thread;
}

uint64_t luaio_get_start_time() {
  return luaio_start_time;
}
