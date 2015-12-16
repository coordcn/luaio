/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 */

#include "init.h"
#include "LuaIO.h"

static ares_channel LuaIO_ares_channel;
static uv_timer_t LuaIO_ares_timer;
static LuaIO_pool_t LuaIO_ares_task_pool;
static LuaIO_hash_t* LuaIO_ares_tasks;

typedef struct {
  UV_HANDLE_FIELDS
  ares_socket_t sock;
  uv_poll_t poll_watcher;
} LuaIO_ares_task_t;

static void LuaIO_ares_timeout(uv_timer_t* handle) {
  ares_process_fd(LuaIO_ares_channel, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
}

static void LuaIO_ares_poll_callback(uv_poll_t* watcher, int status, int events) {
  LuaIO_ares_task_t* task = container_of(watcher, LuaIO_ares_task_t, poll_watcher);

  /* Reset the idle timer */
  uv_timer_again(&LuaIO_ares_timer);

  /* An error happened. Just pretend that the socket is both readable and writable*/
  if (status < 0) {
    ares_process_fd(LuaIO_ares_channel, task->sock, task->sock);
    return;
  }

  /* Process DNS responses */
  ares_process_fd(LuaIO_ares_channel,
                  events & UV_READABLE ? task->sock : ARES_SOCKET_BAD,
                  events & UV_WRITABLE ? task->sock : ARES_SOCKET_BAD);
}

static void LuaIO_ares_poll_close_callback(uv_handle_t* watcher) {
  LuaIO_ares_task_t* task = container_of(watcher, LuaIO_ares_task_t, poll_watcher);
  LuaIO_hash_int_remove(LuaIO_ares_tasks, task->sock);

  if (!LuaIO_ares_tasks->items) {
    uv_timer_stop(&LuaIO_ares_timer);
  }
}

static LuaIO_ares_task_t* LuaIO_ares_task_create(uv_loop_t* loop, ares_socket_t sock) {
  LuaIO_ares_task_t* task = (LuaIO_ares_task_t*) LuaIO_palloc(&LuaIO_ares_task_pool,  
                                                              sizeof(LuaIO_ares_task_t));

  if (!task) {
    /* Out of memory. */
    return NULL;
  }

  task->loop = loop;
  task->sock = sock;

  if (uv_poll_init_socket(loop, &task->poll_watcher, sock) < 0) {
    /* This should never happen. */
    LuaIO_free(task);
    return NULL;
  }

  return task;
}

static void LuaIO_ares_sockstate_callback(void* data,
                                          ares_socket_t sock,
                                          int read,
                                          int write) {
  LuaIO_ares_task_t* task;

  task = (LuaIO_ares_task_t*)LuaIO_hash_int_get(LuaIO_ares_tasks, sock);

  if (read || write) {
    if (!task) {
      /* New socket */

      /* If this is the first socket then start the timer. */
      if (!uv_is_active((uv_handle_t*)(&LuaIO_ares_timer))) {
        uv_timer_start(&LuaIO_ares_timer, LuaIO_ares_timeout, 1000, 1000);
      }

      task = LuaIO_ares_task_create(uv_default_loop(), sock);
      if (!task) {
        /* This should never happen unless we're out of memory or something */
        /* is seriously wrong. The socket won't be polled, but the the query */
        /* will eventually time out. */
        return;
      }

      LuaIO_hash_int_set(LuaIO_ares_tasks, sock, task);
    }
    
     /* This should never fail. If it fails anyway, the query will eventually time out. */
    uv_poll_start(&task->poll_watcher,
                  (read ? UV_READABLE : 0) | (write ? UV_WRITABLE : 0),
                  LuaIO_ares_poll_callback);
  } else {
    /* read == 0 and write == 0 this is c-ares's way of notifying us that */
    /* the socket is now closed. We must free the data associated with socket. */
    uv_close((uv_handle_t*)(&task->poll_watcher), LuaIO_ares_poll_close_callback);
  }
}

static void LuaIO_ares_free_task(void *p) {
  LuaIO_pfree(&LuaIO_ares_task_pool, p);
}

void LuaIO_dns_init(lua_State* L) {
  uv_loop_t* loop = uv_default_loop();
  int ret = ares_library_init(ARES_LIB_INIT_ALL);
  if (ret != ARES_SUCCESS) {
    luaL_error(L, "ares_library_init() error: %s\n", ares_strerror(ret));
    return;
  }

  struct ares_options options;
  LuaIO_memzero(&options, sizeof(options));
  options.flags = ARES_FLAG_NOCHECKRESP;
  options.sock_state_cb = LuaIO_ares_sockstate_callback;
  options.sock_state_cb_data = loop;

  ret = ares_init_options(&LuaIO_ares_channel, 
                          &options, 
                          ARES_OPT_FLAGS | ARES_OPT_SOCK_STATE_CB);
  if (ret != ARES_SUCCESS) {
    luaL_error(L, "ares_init_options() error: %s\n", ares_strerror(ret));
    return;
  }

  uv_timer_init(loop, &LuaIO_ares_timer);
  LuaIO_pool_init(&LuaIO_ares_task_pool, 1024);
  LuaIO_ares_tasks = LuaIO_hash_create_int_pointer(LUAIO_2K_SHIFT,
                                                   0,
                                                   LuaIO_ares_free_task);
}

static void LuaIO_dns_host2addrs(lua_State* L, struct hostent* host) {
  char ip[INET6_ADDRSTRLEN];

  lua_newtable(L);
  for (int i=0; host->h_addr_list[i]; ++i) {
    uv_inet_ntop(host->h_addrtype, host->h_addr_list[i], ip, sizeof(ip));
    lua_pushstring(L, ip);
    lua_rawseti(L, -2, i + 1);
  }
}

static void LuaIO_dns_queryA_callback(void* arg, 
                                      int status, 
                                      int timeouts, 
                                      unsigned char* buf,
                                      int len) {
  lua_State* L = arg;

  if (status != ARES_SUCCESS) {
    lua_pushnil(L);
    lua_pushinteger(L, status);
    LuaIO_resume(L, 2);
    return;
  }

  struct hostent* host;
  int rc = ares_parse_a_reply(buf, len, &host, NULL, NULL);
  if (rc != ARES_SUCCESS) {
    lua_pushnil(L);
    lua_pushinteger(L, rc);
    LuaIO_resume(L, 2);
    return;
  }

  LuaIO_dns_host2addrs(L, host);
  lua_pushinteger(L, 0);
  ares_free_hostent(host);
  LuaIO_resume(L, 2);
}

static int LuaIO_dns_queryA(lua_State* L) {
  const char* name = luaL_checkstring(L, 1);

  ares_query(LuaIO_ares_channel,
             name,
             ns_c_in,
             ns_t_a,
             LuaIO_dns_queryA_callback,
             L);

  return lua_yield(L, 0);
}

static void LuaIO_dns_queryAaaa_callback(void* arg, 
                                         int status, 
                                         int timeouts, 
                                         unsigned char* buf,
                                         int len) {
  lua_State* L = arg;

  if (status != ARES_SUCCESS) {
    lua_pushnil(L);
    lua_pushinteger(L, status);
    LuaIO_resume(L, 2);
    return;
  }

  struct hostent* host;
  int rc = ares_parse_aaaa_reply(buf, len, &host, NULL, NULL);
  if (rc != ARES_SUCCESS) {
    lua_pushnil(L);
    lua_pushinteger(L, rc);
    LuaIO_resume(L, 2);
    return;
  }

  LuaIO_dns_host2addrs(L, host);
  lua_pushinteger(L, 0);
  ares_free_hostent(host);
  LuaIO_resume(L, 2);
}

static int LuaIO_dns_queryAaaa(lua_State* L) {
  const char* name = luaL_checkstring(L, 1);

  ares_query(LuaIO_ares_channel,
             name,
             ns_c_in,
             ns_t_aaaa,
             LuaIO_dns_queryAaaa_callback,
             L);

  return lua_yield(L, 0);
}

int luaopen_dns(lua_State *L){
  luaL_Reg lib[] = {
    { "resolve4", LuaIO_dns_queryA },
    { "resolve6", LuaIO_dns_queryAaaa },
    { "__newindex", LuaIO_cannot_change},
    { NULL, NULL}
  };

  lua_createtable(L, 0, 0);

  luaL_newlib(L, lib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected.");
  lua_setfield(L, -2, "__metatable");

  lua_setmetatable(L, -2);

  return 1;
}
