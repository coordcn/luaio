/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "init.h"
#include "LuaIO.h"

typedef struct {
  int cpu;
  int options_ref;
  uv_process_options_t* options;
} LuaIO_process_data_t;

static char LuaIO_process_exepath[PATH_MAX * 2];
static char* LuaIO_process_exepath_ptr;
static uv_signal_t LuaIO_process_signal;
static uv_signal_t* LuaIO_process_signal_ptr;
static uv_stdio_container_t LuaIO_process_stdio[3];
static uv_stdio_container_t* LuaIO_process_stdio_ptr;

static int LuaIO_process_uptime(lua_State* L) {
  uint64_t uptime;
  uv_loop_t* loop = uv_default_loop();

  uv_update_time(loop);
  uptime = uv_now(loop) - LuaIO_get_start_time();
  lua_pushinteger(L, uptime);

  return 1;
}

static int LuaIO_set_affinity(int pid, int cpu) {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu, &mask);
  return sched_setaffinity(pid, sizeof(cpu_set_t), &mask); 
}

static void LuaIO_process_free_options(uv_process_options_t* options) {
  free(options->args);
  free(options);
}

static void LuaIO_process_signal_callback(uv_signal_t* handle, int signal) {
  /*do nothing, only for maintaining event loop.*/
}

static void LuaIO_process_onexit(uv_process_t* handle, int64_t status, int signal) {
  if (handle->data) {
    LuaIO_process_data_t* data = handle->data;
    fprintf(stderr,
            "process[%d] exit. status: %" PRId64 ", signal: %d, file: %s\n",
            handle->pid,
            status,
            signal,
            data->options->args[1]);

    data->options->file = LuaIO_process_exepath_ptr;
    data->options->args[0] = LuaIO_process_exepath_ptr;

    uv_process_t* process = (uv_process_t*) LuaIO_malloc(sizeof(uv_process_t));
    if (!process) {
      LuaIO_process_free_options(data->options);
      luaL_unref(LuaIO_get_main_thread(), LUA_REGISTRYINDEX, data->options_ref);
      LuaIO_free(data);
      fprintf(stderr,
              "process[%s] restart failed. no more memory for process\n", 
              data->options->args[1]);
    }
    LuaIO_memzero(process, sizeof(uv_process_t));
    process->data = data;

    int ret = uv_spawn(uv_default_loop(), process, data->options);
    if (ret < 0) {
      LuaIO_process_free_options(data->options);
      luaL_unref(LuaIO_get_main_thread(), LUA_REGISTRYINDEX, data->options_ref);
      LuaIO_free(data);
      uv_close((uv_handle_t*)process, NULL);
      fprintf(stderr,
              "process[%s] restart failed. uv_spawn() error: %s\n",
              data->options->args[1],
              uv_strerror(ret));
    }

    if (data->cpu >= 0) LuaIO_set_affinity(process->pid, data->cpu);

    return;
  }
    
  fprintf(stderr, 
          "process[%d] exit. status: %" PRId64 ", signal: %d\n",
          handle->pid,
          status,
          signal);
  
  uv_close((uv_handle_t*)handle, NULL);
}

static int LuaIO_process_fork(lua_State* L) {
  if (lua_type(L, 1) != LUA_TTABLE) {
    return luaL_argerror(L, 1, "process.fork(options) error: options must be [table]\n"); 
  }

  if (!LuaIO_process_exepath_ptr) {
    size_t length = sizeof(LuaIO_process_exepath);
    int err = uv_exepath(LuaIO_process_exepath, &length);
    if (err < 0) {
      return luaL_error(L, "process.fork(options) uv_exepath() error: %s\n", uv_strerror(err));
    }

    LuaIO_process_exepath_ptr = LuaIO_process_exepath;
  }

  lua_getfield(L, 1, "file");
  const char* file;
  if (lua_type(L, -1) == LUA_TSTRING) {
    file = lua_tostring(L, -1);
  } else {
    return luaL_argerror(L, 2, "process.fork(options) error: options.file is required and must be [string]\n"); 
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "args");
  size_t args_length = 0;
  char** args = NULL;
  if (lua_type(L, -1) == LUA_TTABLE) {
    args_length = lua_rawlen(L, -1);
    /*execpath, file, null => args_length + 3*/
    args = LuaIO_malloc(sizeof(char*) * (args_length + 3));
    if (!args) {
      return luaL_error(L, "process.fork(options) error: no memory for args\n");
    }

    for (size_t i = 1; i <= args_length; ++i) {
      lua_rawgeti(L, -1, i);
      args[i + 1] = (char*)luaL_checkstring(L, -1);
      lua_pop(L, 1);
    }
  } else if (lua_type(L, -1) == LUA_TNIL) {
    /*execpath, file, null => args_length + 3*/
    args = LuaIO_malloc(sizeof(char*) * 3);
    if (!args) {
      return luaL_error(L, "process.fork(options) error: no memory for args\n");
    }
  } else {
    return luaL_argerror(L, 2, "process.fork(options) error: options.args must be [table]\n"); 
  }
  lua_pop(L, 1);

  args[0] = LuaIO_process_exepath_ptr;
  args[1] = (char*)file;
  args[args_length + 2] = NULL;

  if (!LuaIO_process_stdio_ptr) { 
    LuaIO_process_stdio[0].flags = UV_INHERIT_FD;
    LuaIO_process_stdio[0].data.fd = 0;
    LuaIO_process_stdio[1].flags = UV_INHERIT_FD;
    LuaIO_process_stdio[1].data.fd = 1;
    LuaIO_process_stdio[2].flags = UV_INHERIT_FD;
    LuaIO_process_stdio[2].data.fd = 2;
    LuaIO_process_stdio_ptr = LuaIO_process_stdio;
  }

  uv_process_options_t* options = LuaIO_malloc(sizeof(uv_process_options_t));
  if (!options) {
    LuaIO_free(args);
    return luaL_error(L, "process.fork(options) error: no memory for uv_process_options_t options\n");
  }
  LuaIO_memzero(options, sizeof(uv_process_options_t));
  options->exit_cb = LuaIO_process_onexit;
  options->file = LuaIO_process_exepath_ptr;
  options->args = args;
  options->stdio_count = 3;
  options->stdio = LuaIO_process_stdio_ptr;

  lua_getfield(L, 1, "uid");
  if (lua_type(L, -1) == LUA_TNUMBER) {
    options->uid = lua_tointeger(L, -1);
    options->flags |= UV_PROCESS_SETUID;
  } else if (lua_type(L, -1) != LUA_TNIL) {
    LuaIO_process_free_options(options);
    return luaL_argerror(L, 2, "process.fork(options) error: options.uid must be [number]\n"); 
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "gid");
  if (lua_type(L, -1) == LUA_TNUMBER) {
    options->gid = lua_tointeger(L, -1);
    options->flags |= UV_PROCESS_SETGID;
  } else if (lua_type(L, -1) != LUA_TNIL) {
    LuaIO_process_free_options(options);
    return luaL_argerror(L, 2, "process.fork(options) error: options.gid must be [number]\n"); 
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "detached");
  if (lua_toboolean(L, -1)) {
    options->flags |= UV_PROCESS_DETACHED;
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "forever");
  int forever = lua_toboolean(L, -1);
  lua_pop(L, 1);
  
  int cpu = -1; 
  lua_getfield(L, 1, "cpu");
  if (lua_type(L, -1) == LUA_TNUMBER) {
    cpu = lua_tointeger(L, -1);
  } else if (lua_type(L, -1) != LUA_TNIL) {
    LuaIO_process_free_options(options);
    return luaL_argerror(L, 2, "process.fork(options) error: options.cpu must be [number]\n"); 
  }
  lua_pop(L, 1);

  uv_process_t* handle = (uv_process_t*)LuaIO_malloc(sizeof(uv_process_t));
  if (!handle) {
    LuaIO_process_free_options(options);
    return luaL_error(L, "process.fork(options) error: no memory for uv_process_t handle.\n");
  }
  LuaIO_memzero(handle, sizeof(uv_process_t));

  LuaIO_process_data_t* data = NULL;
  if (forever) {
    data = (LuaIO_process_data_t*)LuaIO_malloc(sizeof(LuaIO_process_data_t));
    if (!data) {
      LuaIO_process_free_options(options);
      return luaL_error(L, "process.fork(options) error:no memory for LuaIO_process_data_t data.\n");
    }
    
    lua_pushvalue(L, 1);
    data->options_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    data->options = options;
    data->cpu = cpu;
    handle->data = data;
  }

  int ret = uv_spawn(uv_default_loop(), handle, options);
  if (ret < 0) {
    LuaIO_process_free_options(options);
    luaL_unref(L, LUA_REGISTRYINDEX, data->options_ref);
    LuaIO_free(data);
    uv_close((uv_handle_t*)handle, NULL);
    return luaL_error(L, "process.fork(options) uv_spawn() error: %s\n", uv_strerror(ret));
  }
  if (cpu >= 0) LuaIO_set_affinity(handle->pid, cpu);

  if (!LuaIO_process_signal_ptr) {
    uv_signal_init(uv_default_loop(), &LuaIO_process_signal);
    LuaIO_process_signal_ptr = &LuaIO_process_signal;
    uv_signal_start(LuaIO_process_signal_ptr, LuaIO_process_signal_callback, SIGQUIT);
  }

  lua_pushinteger(L, handle->pid);

  return 1;
}

static int LuaIO_process_exec(lua_State* L) {
  const char* cmd;
  if (lua_type(L, 1) == LUA_TSTRING) {
    cmd = lua_tostring(L, 1);
  } else {
    return luaL_argerror(L, 2, "process.exec(cmd) error: cmd is required and must be [string]\n"); 
  }

  /*shell, -c, cmd, null*/
  char** args = LuaIO_malloc(sizeof(char*) * 4);
  args[0] = "/bin/sh";
  args[1] = "-c";
  args[2] = (char*)cmd;
  args[3] = NULL;

  if (!LuaIO_process_stdio_ptr) { 
    LuaIO_process_stdio[0].flags = UV_INHERIT_FD;
    LuaIO_process_stdio[0].data.fd = 0;
    LuaIO_process_stdio[1].flags = UV_INHERIT_FD;
    LuaIO_process_stdio[1].data.fd = 1;
    LuaIO_process_stdio[2].flags = UV_INHERIT_FD;
    LuaIO_process_stdio[2].data.fd = 2;
    LuaIO_process_stdio_ptr = LuaIO_process_stdio;
  }

  uv_process_options_t* options = (uv_process_options_t*)LuaIO_malloc(sizeof(uv_process_options_t));
  if (!options) {
    LuaIO_free(args);
    return luaL_error(L, "process.exec(cmd) error: no memory for uv_process_options_t options\n");
  }
  LuaIO_memzero(options, sizeof(uv_process_options_t));
  options->exit_cb = LuaIO_process_onexit;
  options->file = "/bin/sh";
  options->args = args;
  options->stdio_count = 3;
  options->stdio = LuaIO_process_stdio_ptr;

  uv_process_t* handle = (uv_process_t*)LuaIO_malloc(sizeof(uv_process_t));
  if (!handle) {
    LuaIO_process_free_options(options);
    return luaL_error(L, "process.exec(cmd) error: no memory for uv_process_t handle\n");
  }
  LuaIO_memzero(handle, sizeof(uv_process_t));

  int ret = uv_spawn(uv_default_loop(), handle, options);
  if (ret < 0) {
    LuaIO_process_free_options(options);
    uv_close((uv_handle_t*)handle, NULL);
    return luaL_error(L, "process.exec(cmd) uv_spawn() error: %s\n", uv_strerror(ret));
  }

  if (!LuaIO_process_signal_ptr) {
    uv_signal_init(uv_default_loop(), &LuaIO_process_signal);
    LuaIO_process_signal_ptr = &LuaIO_process_signal;
    uv_signal_start(LuaIO_process_signal_ptr, LuaIO_process_signal_callback, SIGQUIT);
  }

  lua_pushinteger(L, handle->pid);

  return 1;
}

static int LuaIO_process_cwd(lua_State* L) {
  char buf[PATH_MAX];
  size_t length = sizeof(buf);

  int err = uv_cwd(buf, &length);
  if (err < 0) return luaL_error(L, "process.cwd() uv_cwd() error: %s\n", uv_strerror(err));
  lua_pushlstring(L, buf, strlen(buf));

  return 1;
}

static int LuaIO_process_abort(lua_State* L) {
  abort();
  return 0;
}

static int LuaIO_process_exit(lua_State* L) {
  int argc = lua_gettop(L);
  int signal = SIGTERM;

  if (argc == 1) signal = luaL_checkinteger(L, 1);
  exit(signal);

  return 0;
}

static int LuaIO_process_kill(lua_State* L) {
  int pid = luaL_checkinteger(L, 1);
  int argc = lua_gettop(L);
  int signal = SIGTERM;

  if(argc == 2) signal = luaL_checkinteger(L, 2);
  int err = uv_kill(pid, signal);
  if (err < 0) return luaL_error(L, "process.kill(pid[, signal]) uv_kill() error: %s\n", uv_strerror(err));

  return 0;
}

static void LuaIO_process_setup_constants(lua_State* L) {
#ifdef SIGHUP
  lua_pushinteger(L, SIGHUP);
  lua_setfield(L, -2, "SIGHUP");
#endif
#ifdef SIGINT
  lua_pushinteger(L, SIGINT);
  lua_setfield(L, -2, "SIGINT");
#endif
#ifdef SIGQUIT
  lua_pushinteger(L, SIGQUIT);
  lua_setfield(L, -2, "SIGQUIT");
#endif
#ifdef SIGILL
  lua_pushinteger(L, SIGILL);
  lua_setfield(L, -2, "SIGILL");
#endif
#ifdef SIGTRAP
  lua_pushinteger(L, SIGTRAP);
  lua_setfield(L, -2, "SIGTRAP");
#endif
#ifdef SIGABRT
  lua_pushinteger(L, SIGABRT);
  lua_setfield(L, -2, "SIGABRT");
#endif
#ifdef SIGIOT
  lua_pushinteger(L, SIGIOT);
  lua_setfield(L, -2, "SIGIOT");
#endif
#ifdef SIGBUS
  lua_pushinteger(L, SIGBUS);
  lua_setfield(L, -2, "SIGBUS");
#endif
#ifdef SIGFPE
  lua_pushinteger(L, SIGFPE);
  lua_setfield(L, -2, "SIGFPE");
#endif
#ifdef SIGKILL
  lua_pushinteger(L, SIGKILL);
  lua_setfield(L, -2, "SIGKILL");
#endif
#ifdef SIGUSR1
  lua_pushinteger(L, SIGUSR1);
  lua_setfield(L, -2, "SIGUSR1");
#endif
#ifdef SIGSEGV
  lua_pushinteger(L, SIGSEGV);
  lua_setfield(L, -2, "SIGSEGV");
#endif
#ifdef SIGUSR2
  lua_pushinteger(L, SIGUSR2);
  lua_setfield(L, -2, "SIGUSR2");
#endif
#ifdef SIGPIPE
  lua_pushinteger(L, SIGPIPE);
  lua_setfield(L, -2, "SIGPIPE");
#endif
#ifdef SIGALRM
  lua_pushinteger(L, SIGALRM);
  lua_setfield(L, -2, "SIGALRM");
#endif
#ifdef SIGTERM
  lua_pushinteger(L, SIGTERM);
  lua_setfield(L, -2, "SIGTERM");
#endif
#ifdef SIGCHLD
  lua_pushinteger(L, SIGCHLD);
  lua_setfield(L, -2, "SIGCHLD");
#endif
#ifdef SIGSTKFLT
  lua_pushinteger(L, SIGSTKFLT);
  lua_setfield(L, -2, "SIGSTKFLT");
#endif
#ifdef SIGCONT
  lua_pushinteger(L, SIGCONT);
  lua_setfield(L, -2, "SIGCONT");
#endif
#ifdef SIGSTOP
  lua_pushinteger(L, SIGSTOP);
  lua_setfield(L, -2, "SIGSTOP");
#endif
#ifdef SIGTSTP
  lua_pushinteger(L, SIGTSTP);
  lua_setfield(L, -2, "SIGTSTP");
#endif
#ifdef SIGBREAK
  lua_pushinteger(L, SIGBREAK);
  lua_setfield(L, -2, "SIGBREAK");
#endif
#ifdef SIGTTIN
  lua_pushinteger(L, SIGTTIN);
  lua_setfield(L, -2, "SIGTTIN");
#endif
#ifdef SIGTTOU
  lua_pushinteger(L, SIGTTOU);
  lua_setfield(L, -2, "SIGTTOU");
#endif
#ifdef SIGURG
  lua_pushinteger(L, SIGURG);
  lua_setfield(L, -2, "SIGURG");
#endif
#ifdef SIGXCPU
  lua_pushinteger(L, SIGXCPU);
  lua_setfield(L, -2, "SIGXCPU");
#endif
#ifdef SIGXFSZ
  lua_pushinteger(L, SIGXFSZ);
  lua_setfield(L, -2, "SIGXFSZ");
#endif
#ifdef SIGVTALRM
  lua_pushinteger(L, SIGVTALRM);
  lua_setfield(L, -2, "SIGVTALRM");
#endif
#ifdef SIGPROF
  lua_pushinteger(L, SIGPROF);
  lua_setfield(L, -2, "SIGPROF");
#endif
#ifdef SIGWINCH
  lua_pushinteger(L, SIGWINCH);
  lua_setfield(L, -2, "SIGWINCH");
#endif
#ifdef SIGIO
  lua_pushinteger(L, SIGIO);
  lua_setfield(L, -2, "SIGIO");
#endif
#ifdef SIGPOLL
  lua_pushinteger(L, SIGPOLL);
  lua_setfield(L, -2, "SIGPOLL");
#endif
#ifdef SIGLOST
  lua_pushinteger(L, SIGLOST);
  lua_setfield(L, -2, "SIGLOST");
#endif
#ifdef SIGPWR
  lua_pushinteger(L, SIGPWR);
  lua_setfield(L, -2, "SIGPWR");
#endif
#ifdef SIGSYS
  lua_pushinteger(L, SIGSYS);
  lua_setfield(L, -2, "SIGSYS");
#endif
}

int luaopen_process(lua_State *L) {
  luaL_Reg lib[] = {
    { "uptime", LuaIO_process_uptime},
    { "fork", LuaIO_process_fork},
    { "exec", LuaIO_process_exec},
    { "cwd", LuaIO_process_cwd},
    { "abort", LuaIO_process_abort},
    { "exit", LuaIO_process_exit},
    { "kill", LuaIO_process_kill},
    { "__newindex", LuaIO_cannot_change},
    { NULL, NULL}
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
