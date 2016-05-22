/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "lualib.h"
#include "luaio.h"
#include "luaio_init.h"
#include "luaio_pmemory.h"

/*fprintf(stderr, "malloc(size: %" PRId64 ") failed.\n", size);*/

static char *bootstrap =
  "_G.system = require('system')\n"

  "local process_native = require('process_native')\n"
  "local execpath = process_native.execpath()\n"

  "local exec_split_reg = '^(.*)/([^/]*)(/*)$'\n"
  "if system.type == 'Windows' then\n"
  "  exec_split_reg = '^(.*)[/\\\\]([^/\\\\]*)([/\\\\]*)$'\n"
  "end\n"
  "local dir, name = execpath:match(exec_split_reg)\n"
  "package.path = dir .. '/lib/?.lua'\n"
  "package.cpath = ''\n"

  "_G.process = require('process')\n"
  "local path = require('path')\n"
  "local Module = require('module')\n"

  "__LUAIO_BASE_COROUTINE__ = coroutine.create(function()\n"
  "  local file = path.resolve(__ARGV__[2])\n"
  "  local package_file = path.resolve(__ARGV__[3] or 'package.lua')\n"

  "  local package\n"
  "  local package_fn = loadfile(package_file)\n"

  "  if package_fn then\n"
  "    package = package_fn()\n"
  "  end\n"
  
  "  if type(package) ~= 'table' then\n"
  "    package = {}\n"
  "  end\n"
 
  "  local module = Module:new(file, package)\n"
  "  module:require(file)\n"
  "end)\n"
  
  "coroutine.resume(__LUAIO_BASE_COROUTINE__)\n"
;

#if LUA_VERSION_NUM == 503

static void *luaio_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud; (void)osize;  /* not used */
  return luaio_prealloc(ptr, nsize);
}

#endif

static int luaio_panic (lua_State *L) {
  lua_writestringerror(LUAIO_COLOR_ERROR
                       "PANIC: unprotected error in call to Lua API\n%s\n"
                       LUAIO_COLOR_RESET,
                       lua_tostring(L, -1));
  return 0;  /* return to Lua to abort */
}

int main(int argc, char *argv[]) {
  argv = uv_setup_args(argc, argv);

  luaio_pmemory_init();

#if LUA_VERSION_NUM < 502
  lua_State *L = luaL_newstate();
#else
  lua_State *L = lua_newstate(luaio_alloc, NULL);
#endif
  if (L == NULL) {
    fprintf(stderr,
            LUAIO_COLOR_ERROR
            "lua_newstate() error: no memory for main thread\n"
            LUAIO_COLOR_RESET);
    return 1;
  }
  
  lua_atpanic(L, &luaio_panic);
  luaL_openlibs(L);

  if (luaio_init(L, argc, argv)) {
    fprintf(stderr,
            LUAIO_COLOR_ERROR
            "luaio_init(L) failed\n"
            LUAIO_COLOR_RESET);
    return 1;
  }

  if (luaL_dostring(L, bootstrap)) {
    fprintf(stderr,
            LUAIO_COLOR_ERROR
            "%s\n"
            LUAIO_COLOR_RESET, 
            lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_close(L);
    return -1;
  }

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  lua_close(L);

  return 0;
}
