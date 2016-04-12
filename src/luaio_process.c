/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 */

#include "luaio.h"
#include "luaio_init.h"
#include "luaio_setaffinity.h"

typedef struct {
  lua_State     *current_thread;
  uv_process_t  handle;
  int           onexit_ref;
} luaio_process_t;

static uv_stdio_container_t luaio_process_stdio[3];

/* @brief: return number of seconds the process has been running
 * @example: local uptime = process_native.uptime()
 * @return: uptime {integer}
 */
static int luaio_process_uptime(lua_State *L) {
  uv_loop_t *loop = uv_default_loop();
  uv_update_time(loop);
  uint64_t uptime = uv_now(loop) - luaio_get_start_time();

  lua_pushinteger(L, uptime);
  return 1;
}

/* @brief: set cpu affinity of a process
 * @example: local ret = process_native.setaffinity(pid, cpu_id)
 * @param: pid {integer}
 * @param: cpu_id {integer}
 * @return: ret {integer} if ret < 0 => error
 */
static int luaio_process_setaffinity(lua_State *L) {
  int pid = luaL_checkinteger(L, 1);
  int cpu_id = luaL_checkinteger(L, 2);
  int ret = luaio_setaffinity(pid, cpu_id);

  lua_pushinteger(L, ret);
  return 1;
}

/* @brief: set process title
 * @example: local ret = process_native.settitle(title)
 * @param: title {integer}
 * @return: ret {integer} if ret < 0 => error
 */
static int luaio_process_settitle(lua_State *L) {
  const char *title = luaL_checkstring(L, 1);
  int ret = uv_set_process_title(title);
  lua_pushinteger(L, ret);
  return 1;
}

/* @brief: get process title
 * @example: local title = process_native.gettitle()
 * @return: title {integer}
 */
static int luaio_process_gettitle(lua_State *L) {
  char buf[512];
  int ret = uv_get_process_title(buf, sizeof(buf));
  if (ret < 0) {
    lua_pushnil(L);
  } else {
    lua_pushstring(L, buf);
  }

  return 1;
}

static void luaio_process_onclose(uv_handle_t *handle) {
  luaio_process_t *process = container_of(handle, luaio_process_t, handle);
  
  int onexit_ref = process->onexit_ref;
  if (onexit_ref != LUA_NOREF) {
    lua_State *L = process->current_thread;
    luaL_unref(L, LUA_REGISTRYINDEX, onexit_ref);
  }

  luaio_free(process);
}

static void luaio_process_onexit(uv_process_t *handle, int64_t status, int signal) {
  luaio_process_t *process = container_of(handle, luaio_process_t, handle);
  
  int onexit_ref = process->onexit_ref;
  if (onexit_ref != LUA_NOREF) {
    lua_State *L = process->current_thread;
    lua_rawgeti(L, LUA_REGISTRYINDEX, onexit_ref);
    lua_pushinteger(L, handle->pid);
    lua_pushinteger(L, status);
    lua_pushinteger(L, signal);
    luaio_pcall(L, 3);
  }

  uv_close((uv_handle_t*)handle, luaio_process_onclose);
}

/* @brief: launch a new process with the given options
 * @example: local ret = process_native.spawn(options)
 * @param: options {table}
 *    local table = {
 *      file = {string}
 *      args = {table[array(string)]}
 *      onexit = {function}
 *      uid = {integer}
 *      gid = {integer}
 *      detached = {boolean}
 *    }
 *
 *    @param: pid {integer}
 *    @param: status {integer}
 *    @param: signal {integer}
 *    function onexit(pid, status, signal)
 *    end
 * @return: ret {integer} 
 *    if ret < 0 => error
 *    if ret > 0 => pid
 */
static int luaio_process_spawn(lua_State *L) {
  uv_process_options_t options;
  luaio_memzero(&options, sizeof(uv_process_options_t));
  options.stdio_count = 3;
  options.stdio = luaio_process_stdio;
  options.exit_cb = luaio_process_onexit;

  /*file*/
  lua_getfield(L, 1, "file");
  options.file = lua_tostring(L, -1);
  lua_pop(L, 1);

  /*args*/
  lua_getfield(L, 1, "args");
  size_t argc = lua_rawlen(L, -1);

  char **args = luaio_malloc(sizeof(char*) * (argc + 1));
  if (!args) {
    lua_pushinteger(L, UV_ENOMEM);
    return 1;
  }

  for (size_t i = 0; i < argc; i++) {
    lua_rawgeti(L, -1, i + 1);
    args[i] = (char*)lua_tostring(L, -1);
    lua_pop(L, 1);
  }

  args[argc] = NULL;
  options.args = args;
  lua_pop(L, 1);

  /*uid*/
  lua_getfield(L, 1, "uid");
  if (lua_type(L, -1) == LUA_TNUMBER) {
    options.uid = lua_tointeger(L, -1);
    options.flags |= UV_PROCESS_SETUID;
  }
  lua_pop(L, 1);

  /*gid*/
  lua_getfield(L, 1, "gid");
  if (lua_type(L, -1) == LUA_TNUMBER) {
    options.gid = lua_tointeger(L, -1);
    options.flags |= UV_PROCESS_SETGID;
  }
  lua_pop(L, 1);

  /*detached*/
  lua_getfield(L, 1, "detached");
  if (lua_toboolean(L, -1)) {
    options.flags |= UV_PROCESS_DETACHED;
  }
  lua_pop(L, 1);

  /*onexit*/
  lua_getfield(L, 1, "onexit");
  int onexit_ref = LUA_NOREF;
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    onexit_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  luaio_process_t *process = luaio_malloc(sizeof(luaio_process_t));
  if (!process) {
    luaio_free(args);
    if (onexit_ref != LUA_NOREF) {
      luaL_unref(L, LUA_REGISTRYINDEX, onexit_ref);
    }

    lua_pushinteger(L, UV_ENOMEM);
    return 1;
  }
  process->current_thread = L;
  process->onexit_ref = onexit_ref;

  uv_process_t *handle = &process->handle;
  int ret = uv_spawn(uv_default_loop(), handle, &options);
  luaio_free(args);

  if (ret < 0) {
    uv_close((uv_handle_t*)handle, luaio_process_onclose);
    lua_pushinteger(L, ret);
    return 1;
  }

  lua_pushinteger(L, handle->pid);
  return 1;
}

/* @brief: return the current working directory of the process
 * @example: local cwd = process_native.cwd()
 * @return: cwd {string|nil}
 */
static int luaio_process_cwd(lua_State *L) {
#ifdef _WIN32
  /* MAX_PATH is in characters, not bytes. Make sure we have enough headroom. */
  char buf[MAX_PATH * 4];
#else
  char buf[PATH_MAX];
#endif

  size_t size = sizeof(buf);
  int ret = uv_cwd(buf, &size);
  if (ret < 0) {
    lua_pushnil(L);
  } else {
    lua_pushlstring(L, buf, size);
  }
  return 1;
}

/* @brief: return the absolute pathname of the executable that started the process
 *        example: /usr/local/bin/luaio
 * @example: local execpath = process_native.execpath()
 * @return: execpath {string|nil}
 */
static int luaio_process_execpath(lua_State *L) {
  char buf[PATH_MAX * 2];
  size_t size = sizeof(buf);
  int ret = uv_exepath(buf, &size);
  if (ret < 0) {
    lua_pushnil(L);
  } else {
    lua_pushlstring(L, buf, size);
  }
  return 1;
}

/* @brief: abort the process and generate a core file(if core file is open)
 * @example: process_native.abort()
 */
static int luaio_process_abort(lua_State* L) {
  abort();
  return 0;
}

/* @brief: end the process with specifyed code
 * @example: process_native.exit([code])
 * @param: code {integer|default: 0}
 *   if code == 0 => success
 *   if code != 0 => failure
 */
static int luaio_process_exit(lua_State *L) {
  int code = luaL_opt(L, luaL_checkinteger, 1, 0);
  exit(code);
  return 0;
}

/* @brief: send a signal to a process 
 * @example: local ret = process_native.kill(pid[, signal])
 * @param: pid {integer}
 * @param: signal {integer|default: SIGTERM}
 * @return: ret {integer}
 */
static int luaio_process_kill(lua_State *L) {
  int pid = luaL_checkinteger(L, 1);
  int signal = luaL_opt(L, luaL_checkinteger, 2, SIGTERM);
  int ret = uv_kill(pid, signal);
  lua_pushinteger(L, ret);
  return 1;
}

int luaopen_process(lua_State *L) {
  luaio_process_stdio[0].flags = UV_INHERIT_FD;
  luaio_process_stdio[0].data.fd = 0;
  luaio_process_stdio[1].flags = UV_INHERIT_FD;
  luaio_process_stdio[1].data.fd = 1;
  luaio_process_stdio[2].flags = UV_INHERIT_FD;
  luaio_process_stdio[2].data.fd = 2;

  luaL_Reg lib[] = {
    { "uptime", luaio_process_uptime },
    { "setaffinity", luaio_process_setaffinity },
    { "settitle", luaio_process_settitle },
    { "gettitle", luaio_process_gettitle },
    { "spawn", luaio_process_spawn },
    { "cwd", luaio_process_cwd },
    { "execpath", luaio_process_execpath },
    { "abort", luaio_process_abort },
    { "exit", luaio_process_exit },
    { "kill", luaio_process_kill },
    { "__newindex", luaio_cannot_change },
    { NULL, NULL }
  };

  lua_createtable(L, 0, 0);

  luaL_newlib(L, lib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected.");
  lua_setfield(L, -2, "__metatable");
  lua_pushinteger(L, getpid());
  lua_setfield(L, -2, "pid");

  lua_setmetatable(L, -2);

  return 1;
}
