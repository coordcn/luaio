/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "init.h"
#include "lualib.h"

int main(int argc, char *argv[]) {
  lua_State *L;
  argv = uv_setup_args(argc, argv);
  
  L = luaL_newstate();
  if (L == NULL) {
    fprintf(stderr, "no more memory for main thread\n");
    return 1;
  }
  
  luaL_openlibs(L);

  if (LuaIO_init(L, argc, argv)) {
    fprintf(stderr, "LuaIO_init(L) failed\n");
    return 1;
  }

  const char* fname = argv[1];
  if (luaL_dofile(L, fname)) {
    printf("%s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_close(L);
    return -1;
  }
  
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  lua_close(L);
  return 0;
}
