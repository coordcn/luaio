/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio.h"
#include "luaio_init.h"

typedef struct {
  lua_State     *current_thread;
  uv_signal_t   handle;
  int           callback_ref;
} luaio_signal_t;

static char luaio_signal_metatable_key;

#define luaio_signal_check_signal(L, name) \
  luaio_signal_t **signal_ptr = lua_touserdata(L, 1); \
  if (signal_ptr == NULL) { \
    return luaL_argerror(L, 1, "signal:"#name" error: signal must be userdata\n"); \
  } \
  \
  luaio_signal_t *signal = *signal_ptr; \
  if (signal == NULL) { \
    return luaL_argerror(L, 1, "signal:"#name" error: signal memory has been released\n"); \
  }

/* @brief: create new signal object
 * @warning: must be used in main thread
 * @example: local signal = signal.new()
 */
static int luaio_signal_new(lua_State *L) {
  luaio_signal_t **signal_ptr = lua_newuserdata(L, sizeof(luaio_signal_t*));
  if (signal_ptr == NULL) {
    lua_pushnil(L);
    return 1;
  }

  luaio_signal_t *signal = luaio_palloc(sizeof(luaio_signal_t));
  if (signal == NULL) {
    lua_pushnil(L);
    return 1;
  }

  *signal_ptr = signal;

  uv_loop_t *loop = uv_default_loop();
  uv_signal_t *handle = &signal->handle;
  uv_signal_init(loop, handle);
  uv_unref((uv_handle_t*)handle);
  signal->current_thread = L;
  signal->callback_ref = LUA_NOREF;

  lua_pushlightuserdata(L, &luaio_signal_metatable_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_setmetatable(L, -2);
  return 1;
}

static void luaio_signal_callback(uv_signal_t *handle, int signum) {
  luaio_signal_t *signal = container_of(handle, luaio_signal_t, handle);
  lua_State *L = signal->current_thread;
  lua_rawgeti(L, LUA_REGISTRYINDEX, signal->callback_ref);
  luaio_pcall(L, 0);
}

/* @brief: start signal watching
 * @warning: must be used in main thread
 * @example: local ret = signal:start(signum, callback)
 * @param: signum {integer}
 * @param: callback {function}
 * @return: ret {integer} if < 0 => error
 */
static int luaio_signal_start(lua_State *L) {
  luaio_signal_check_signal(L, start(signum, callback));

  int signum = luaL_checkinteger(L, 2);

  if (lua_type(L, 3) != LUA_TFUNCTION) {
    return luaL_argerror(L, 2, "signal:start(signum, callback) error: callback must be function\n"); 
  }

  int callback_ref = signal->callback_ref;
  if (callback_ref != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, callback_ref);
  }

  lua_pushvalue(L, 3);
  signal->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

  int ret = uv_signal_start(&signal->handle, 
                            luaio_signal_callback,
                            signum);

  lua_pushinteger(L, ret);
  return 1;
}

/* @brief: stop signal watching
 * @warning: must be used in main thread
 * @example: local ret = signal:stop()
 */
static int luaio_signal_stop(lua_State *L) {
  luaio_signal_check_signal(L, stop());
  int ret = uv_signal_stop(&signal->handle);
  lua_pushinteger(L, ret);
  return 1;
}

static void luaio_signal_onclose(uv_handle_t *handle) {
  luaio_signal_t *signal = container_of(handle, luaio_signal_t, handle);
  luaio_pfree(signal);
}
/* @brief: close signal handle
 * @warning: must be used in main thread
 * @example: signal:close()
 */
static int luaio_signal_close(lua_State *L) {
  luaio_signal_check_signal(L, close());

  int callback_ref = signal->callback_ref;
  if (callback_ref != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, callback_ref);
  }

  uv_handle_t *handle = (uv_handle_t*)(&signal->handle);
  if (uv_is_closing(handle)) {
    luaL_error(L, "signal:close() error: signal is already closing");
  }

  uv_close(handle, luaio_signal_onclose);
  return 0;
}

static void luaio_signal_setup_constants(lua_State *L) {
#ifdef SIGHUP
  luaio_constant(SIGHUP)
#endif
#ifdef SIGINT
  luaio_constant(SIGINT)
#endif
#ifdef SIGQUIT
  luaio_constant(SIGQUIT)
#endif
#ifdef SIGILL
  luaio_constant(SIGILL)
#endif
#ifdef SIGTRAP
  luaio_constant(SIGTRAP)
#endif
#ifdef SIGABRT
  luaio_constant(SIGABRT)
#endif
#ifdef SIGIOT
  luaio_constant(SIGIOT)
#endif
#ifdef SIGBUS
  luaio_constant(SIGBUS)
#endif
#ifdef SIGFPE
  luaio_constant(SIGFPE)
#endif
#ifdef SIGKILL
  luaio_constant(SIGKILL)
#endif
#ifdef SIGUSR1
  luaio_constant(SIGUSR1)
#endif
#ifdef SIGSEGV
  luaio_constant(SIGSEGV)
#endif
#ifdef SIGUSR2
  luaio_constant(SIGUSR2)
#endif
#ifdef SIGPIPE
  luaio_constant(SIGPIPE)
#endif
#ifdef SIGALRM
  luaio_constant(SIGALRM)
#endif
#ifdef SIGTERM
  luaio_constant(SIGTERM)
#endif
#ifdef SIGCHLD
  luaio_constant(SIGCHLD)
#endif
#ifdef SIGSTKFLT
  luaio_constant(SIGSTKFLT)
#endif
#ifdef SIGCONT
  luaio_constant(SIGCONT)
#endif
#ifdef SIGSTOP
  luaio_constant(SIGSTOP)
#endif
#ifdef SIGTSTP
  luaio_constant(SIGTSTP)
#endif
#ifdef SIGBREAK
  luaio_constant(SIGBREAK)
#endif
#ifdef SIGTTIN
  luaio_constant(SIGTTIN)
#endif
#ifdef SIGTTOU
  luaio_constant(SIGTTOU)
#endif
#ifdef SIGURG
  luaio_constant(SIGURG)
#endif
#ifdef SIGXCPU
  luaio_constant(SIGXCPU)
#endif
#ifdef SIGXFSZ
  luaio_constant(SIGXFSZ)
#endif
#ifdef SIGVTALRM
  luaio_constant(SIGVTALRM)
#endif
#ifdef SIGPROF
  luaio_constant(SIGPROF)
#endif
#ifdef SIGWINCH
  luaio_constant(SIGWINCH)
#endif
#ifdef SIGIO
  luaio_constant(SIGIO)
#endif
#ifdef SIGPOLL
  luaio_constant(SIGPOLL)
#endif
#ifdef SIGLOST
  luaio_constant(SIGLOST)
#endif
#ifdef SIGPWR
  luaio_constant(SIGPWR)
#endif
#ifdef SIGSYS
  luaio_constant(SIGSYS)
#endif
}

/* @brief: parse signal number to uppercase name
 * @example: local signame = signal.parse(signum)
 * @param: signum {integer}
 * @return: signame {string}
 */
static int luaio_signal_parse(lua_State *L) {
#define XX(sig) \
  case sig: \
    signame = #sig; \
    break;

  int signum = luaL_checkinteger(L, 1);
  const char *signame;

  switch (signum) {
#ifdef SIGHUP
    XX(SIGHUP)
#endif
#ifdef SIGINT
    XX(SIGINT)
#endif
#ifdef SIGQUIT
    XX(SIGQUIT)
#endif
#ifdef SIGILL
    XX(SIGILL)
#endif
#ifdef SIGTRAP
    XX(SIGTRAP)
#endif
#ifdef SIGABRT
    XX(SIGABRT)
#endif
#ifdef SIGIOT
  #if SIGIOT != SIGABRT
    XX(SIGIOT)
  #endif
#endif
#ifdef SIGBUS
    XX(SIGBUS)
#endif
#ifdef SIGFPE
    XX(SIGFPE)
#endif
#ifdef SIGKILL
    XX(SIGKILL)
#endif
#ifdef SIGUSR1
    XX(SIGUSR1)
#endif
#ifdef SIGSEGV
    XX(SIGSEGV)
#endif
#ifdef SIGUSR2
    XX(SIGUSR2)
#endif
#ifdef SIGPIPE
    XX(SIGPIPE)
#endif
#ifdef SIGALRM
    XX(SIGALRM)
#endif
#ifdef SIGTERM
    XX(SIGTERM)
#endif
#ifdef SIGCHLD
    XX(SIGCHLD)
#endif
#ifdef SIGSTKFLT
    XX(SIGSTKFLT)
#endif
#ifdef SIGCONT
    XX(SIGCONT)
#endif
#ifdef SIGSTOP
    XX(SIGSTOP)
#endif
#ifdef SIGTSTP
    XX(SIGTSTP)
#endif
#ifdef SIGBREAK
    XX(SIGBREAK)
#endif
#ifdef SIGTTIN
    XX(SIGTTIN)
#endif
#ifdef SIGTTOU
    XX(SIGTTOU)
#endif
#ifdef SIGURG
    XX(SIGURG)
#endif
#ifdef SIGXCPU
    XX(SIGXCPU)
#endif
#ifdef SIGXFSZ
    XX(SIGXFSZ)
#endif
#ifdef SIGVTALRM
    XX(SIGVTALRM)
#endif
#ifdef SIGPROF
    XX(SIGPROF)
#endif
#ifdef SIGWINCH
    XX(SIGWINCH)
#endif
#ifdef SIGIO
    XX(SIGIO)
#endif
#ifdef SIGPOLL
  #if SIGPOLL != SIGIO
    XX(SIGPOLL)
  #endif
#endif
#ifdef SIGLOST
    XX(SIGLOST)
#endif
#ifdef SIGPWR
  #if SIGPWR != SIGLOST
    XX(SIGPWR)
  #endif
#endif
#ifdef SIGSYS
    XX(SIGSYS)
#endif
    default:
      signame = "UNKNOWN";
  }

#undef XX

  lua_pushstring(L, signame);
  return 1;
}

int luaopen_signal(lua_State *L) {
  /*signal metatable*/
  luaL_Reg signal_mtlib[] = {
    { "start", luaio_signal_start },
    { "stop", luaio_signal_stop },
    { "close", luaio_signal_close },
    { "__newindex", luaio_cannot_change },
    { NULL, NULL }
  };

  lua_pushlightuserdata(L, &luaio_signal_metatable_key);
  luaL_newlib(L, signal_mtlib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected");
  lua_setfield(L, -2, "__metatable");
  lua_rawset(L, LUA_REGISTRYINDEX);

  luaL_Reg lib[] = {
    { "new", luaio_signal_new },
    { "parse", luaio_signal_parse },
    { "__newindex", luaio_cannot_change },
    { NULL, NULL }
  };

  lua_createtable(L, 0, 0);

  luaL_newlib(L, lib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected");
  lua_setfield(L, -2, "__metatable");
  luaio_signal_setup_constants(L);

  lua_setmetatable(L, -2);

  return 1;
}
