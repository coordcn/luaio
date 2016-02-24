/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 */

#include "init.h"
#include "LuaIO.h"
#include "setaffinity.h"

typedef struct {
  lua_State     *current_thread;
  uv_process_t  handle;
  int           onexit_ref;
} LuaIO_process_t;

static uv_stdio_container_t LuaIO_process_stdio[3];

/* @brief: return number of seconds the process has been running
 * @example: local uptime = process_native.uptime()
 * @return: uptime {integer}
 */
static int LuaIO_process_uptime(lua_State *L) {
  uv_loop_t *loop = uv_default_loop();
  uv_update_time(loop);
  uint64_t uptime = uv_now(loop) - LuaIO_get_start_time();

  lua_pushinteger(L, uptime);
  return 1;
}

/* @brief: set cpu affinity of a process
 * @example: local ret = process_native.setaffinity(pid, cpu_id)
 * @param: pid {integer}
 * @param: cpu_id {integer}
 * @return: ret {integer} if ret < 0 => error
 */
static int LuaIO_process_setaffinity(lua_State *L) {
  int pid = luaL_checkinteger(L, 1);
  int cpu_id = luaL_checkinteger(L, 2);
  int ret = LuaIO_setaffinity(pid, cpu_id);

  lua_pushinteger(L, ret);
  return 1;
}

static void LuaIO_process_onclose(uv_handle_t *handle) {
  LuaIO_process_t *process = container_of(handle, LuaIO_process_t, handle);
  
  int onexit_ref = process->onexit_ref;
  if (onexit_ref != LUA_NOREF) {
    lua_State *L = process->current_thread;
    luaL_unref(L, LUA_REGISTRYINDEX, onexit_ref);
  }

  LuaIO_free(process);
}

static void LuaIO_process_onexit(uv_process_t *handle, int64_t status, int signal) {
  LuaIO_process_t *process = container_of(handle, LuaIO_process_t, handle);
  
  int onexit_ref = process->onexit_ref;
  if (onexit_ref != LUA_NOREF) {
    lua_State *L = process->current_thread;
    lua_rawgeti(L, LUA_REGISTRYINDEX, onexit_ref);
    lua_pushinteger(L, handle->pid);
    lua_pushinteger(L, status);
    lua_pushinteger(L, signal);
    LuaIO_pcall(L, 3);
  }

  uv_close((uv_handle_t*)handle, LuaIO_process_onclose);
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
static int LuaIO_process_spawn(lua_State *L) {
  uv_process_options_t options;
  LuaIO_memzero(&options, sizeof(uv_process_options_t));
  options.stdio_count = 3;
  options.stdio = LuaIO_process_stdio;
  options.exit_cb = LuaIO_process_onexit;

  /*file*/
  lua_getfield(L, 1, "file");
  options.file = lua_tostring(L, -1);
  lua_pop(L, 1);

  /*args*/
  lua_getfield(L, 1, "args");
  size_t argc = lua_rawlen(L, -1) + 1;

  char **args = LuaIO_malloc(sizeof(char*) * argc);
  if (!args) {
    lua_pushinteger(L, UV_ENOMEM);
    return 1;
  }

  for (size_t i = 1; i < argc; ++i) {
    lua_rawgeti(L, -1, i);
    args[i - 1] = (char*)lua_tostring(L, -1);
    lua_pop(L, 1);
  }

  args[argc] = NULL;
  options.args = args;
  lua_pop(L, 1);

  /*uid*/
  lua_getfield(L, 1, "uid");
  if (lua_type(L, -1) == LUA_TNUMBER) {
    options->uid = lua_tointeger(L, -1);
    options->flags |= UV_PROCESS_SETUID;
  }
  lua_pop(L, 1);

  /*gid*/
  lua_getfield(L, 1, "gid");
  if (lua_type(L, -1) == LUA_TNUMBER) {
    options->gid = lua_tointeger(L, -1);
    options->flags |= UV_PROCESS_SETGID;
  }
  lua_pop(L, 1);

  /*detached*/
  lua_getfield(L, 1, "detached");
  if (lua_toboolean(L, -1)) {
    options->flags |= UV_PROCESS_DETACHED;
  }
  lua_pop(L, 1);

  /*onexit*/
  lua_getfield(L, 1, "onexit");
  type = lua_type(L, -1);
  int onexit_ref = LUA_NOREF;
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    onexit_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  LuaIO_process_t *process = LuaIO_malloc(sizeof(LuaIO_process_t));
  if (!process) {
    LuaIO_free(args);
    if (onexit_ref != LUA_NOREF) {
      luaL_unref(L, LUA_REGISTRYINDEX, onexit_ref);
    }

    lua_pushinteger(L, UV_ENOMEM);
    return 1;
  }
  process->current_thread = L;
  process->onexit_ref = onexit_ref;

  uv_process_t *handle = &process->handle;
  int ret = uv_spawn(uv_default_loop(), handle, options);
  LuaIO_free(args);

  if (ret < 0) {
    uv_close((uv_handle_t*)handle, LuaIO_process_onclose);
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
static int LuaIO_process_cwd(lua_State *L) {
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
 *        example: /usr/local/bin/LuaIO
 * @example: local execpath = process_native.execpath()
 * @return: execpath {string|nil}
 */
static int LuaIO_process_execpath(lua_State *L) {
  char buf[PATH_MAX * 2];
  size_t size = sizeof(buf);
  int ret = uv_exepath(buf, &size);
  if (ret < 0) {
    lua_pushnil(L);
  } else {
    lua_pushlstring(L, buf, size);
  }
  return 1
}

/* @brief: abort the process and generate a core file(if core file is open)
 * @example: process_native.abort()
 */
static int LuaIO_process_abort(lua_State* L) {
  abort();
  return 0;
}

/* @brief: end the process with specifyed code
 * @example: process_native.exit([code])
 * @param: code {integer|default: 0}
 *   if code == 0 => success
 *   if code != 0 => failure
 */
static int LuaIO_process_exit(lua_State *L) {
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
static int LuaIO_process_kill(lua_State *L) {
  int pid = luaL_checkinteger(L, 1);
  int signal = luaL_opt(L, luaL_checkinteger, 2, SIGTERM);
  int ret = uv_kill(pid, signal);
  lua_pushinteger(L, ret);
  return 1;
}

static void LuaIO_process_setup_constants(lua_State *L) {
#ifdef SIGHUP
  LuaIO_constant(SIGHUP)
#endif
#ifdef SIGINT
  LuaIO_constant(SIGINT)
#endif
#ifdef SIGQUIT
  LuaIO_constant(SIGQUIT)
#endif
#ifdef SIGILL
  LuaIO_constant(SIGILL)
#endif
#ifdef SIGTRAP
  LuaIO_constant(SIGTRAP)
#endif
#ifdef SIGABRT
  LuaIO_constant(SIGABRT)
#endif
#ifdef SIGIOT
  LuaIO_constant(SIGIOT)
#endif
#ifdef SIGBUS
  LuaIO_constant(SIGBUS)
#endif
#ifdef SIGFPE
  LuaIO_constant(SIGFPE)
#endif
#ifdef SIGKILL
  LuaIO_constant(SIGKILL)
#endif
#ifdef SIGUSR1
  LuaIO_constant(SIGUSR1)
#endif
#ifdef SIGSEGV
  LuaIO_constant(SIGSEGV)
#endif
#ifdef SIGUSR2
  LuaIO_constant(SIGUSR2)
#endif
#ifdef SIGPIPE
  LuaIO_constant(SIGPIPE)
#endif
#ifdef SIGALRM
  LuaIO_constant(SIGALRM)
#endif
#ifdef SIGTERM
  LuaIO_constant(SIGTERM)
#endif
#ifdef SIGCHLD
  LuaIO_constant(SIGCHLD)
#endif
#ifdef SIGSTKFLT
  LuaIO_constant(SIGSTKFLT)
#endif
#ifdef SIGCONT
  LuaIO_constant(SIGCONT)
#endif
#ifdef SIGSTOP
  LuaIO_constant(SIGSTOP)
#endif
#ifdef SIGTSTP
  LuaIO_constant(SIGTSTP)
#endif
#ifdef SIGBREAK
  LuaIO_constant(SIGBREAK)
#endif
#ifdef SIGTTIN
  LuaIO_constant(SIGTTIN)
#endif
#ifdef SIGTTOU
  LuaIO_constant(SIGTTOU)
#endif
#ifdef SIGURG
  LuaIO_constant(SIGURG)
#endif
#ifdef SIGXCPU
  LuaIO_constant(SIGXCPU)
#endif
#ifdef SIGXFSZ
  LuaIO_constant(SIGXFSZ)
#endif
#ifdef SIGVTALRM
  LuaIO_constant(SIGVTALRM)
#endif
#ifdef SIGPROF
  LuaIO_constant(SIGPROF)
#endif
#ifdef SIGWINCH
  LuaIO_constant(SIGWINCH)
#endif
#ifdef SIGIO
  LuaIO_constant(SIGIO)
#endif
#ifdef SIGPOLL
  LuaIO_constant(SIGPOLL)
#endif
#ifdef SIGLOST
  LuaIO_constant(SIGLOST)
#endif
#ifdef SIGPWR
  LuaIO_constant(SIGPWR)
#endif
#ifdef SIGSYS
  LuaIO_constant(SIGSYS)
#endif
}

int luaopen_process(lua_State *L) {
  LuaIO_process_stdio[0].flags = UV_INHERIT_FD;
  LuaIO_process_stdio[0].data.fd = 0;
  LuaIO_process_stdio[1].flags = UV_INHERIT_FD;
  LuaIO_process_stdio[1].data.fd = 1;
  LuaIO_process_stdio[2].flags = UV_INHERIT_FD;
  LuaIO_process_stdio[2].data.fd = 2;

  luaL_Reg lib[] = {
    { "uptime", LuaIO_process_uptime },
    { "setaffinity", LuaIO_process_setaffinity },
    { "spawn", LuaIO_process_spawn },
    { "cwd", LuaIO_process_cwd },
    { "execpath", LuaIO_process_execpath },
    { "abort", LuaIO_process_abort },
    { "exit", LuaIO_process_exit },
    { "kill", LuaIO_process_kill },
    { "__newindex", LuaIO_cannot_change },
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
  LuaIO_process_setup_constants(L);

  lua_setmetatable(L, -2);

  return 1;
}
