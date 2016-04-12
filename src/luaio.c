/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "lualib.h"
#include "luaio_init.h"
#include "luaio_pmemory.h"

static void *luaio_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud; (void)osize;  /* not used */
  return luaio_prealloc(ptr, nsize);
}

static int luaio_panic (lua_State *L) {
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        lua_tostring(L, -1));
  return 0;  /* return to Lua to abort */
}

int main(int argc, char *argv[]) {
  argv = uv_setup_args(argc, argv);

  luaio_pmemory_init();
  lua_State *L = lua_newstate(luaio_alloc, NULL);
  if (L == NULL) {
    fprintf(stderr, "lua_newstate() error: no memory for main thread\n");
    return 1;
  }
  
  lua_atpanic(L, &luaio_panic);
  luaL_openlibs(L);

  if (luaio_init(L, argc, argv)) {
    fprintf(stderr, "luaio_init(L) failed\n");
    return 1;
  }

  if (luaL_dofile(L, "./lib/bootstrap.lua")) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_close(L);
    return -1;
  }

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  lua_close(L);

  return 0;
}
