/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "init.h"
#include "common.h"

typedef struct {
  size_t          type;
  uint64_t        timeout;
  uv_timer_t      *timer;
  lua_State       *thread;
  lua_State       *current_thread;
  LuaIO_buffer_t  *read_buffer;
  uv_tcp_t        handle;
  int             thread_ref;
  int             onconnect_ref;
  int             write_callback_ref;
} LuaIO_tcp_socket_t;

typedef struct {
  lua_State       *current_thread;
  uv_timer_t      *timer;
  uv_connect_t    req;
} LuaIO_tcp_connect_req_t;

typedef struct {
  lua_State       *current_thread;
  uv_timer_t      *timer;
  int             write_data_ref;
  int             write_callback_ref;
  uv_write_t      req;
} LuaIO_tcp_write_req_t;

static char LuaIO_tcp_socket_metatable_key;

#define LuaIO_tcp_check_socket(L, name) \
  LuaIO_tcp_socket_t *socket = lua_touserdata(L, 1); \
  if (socket == NULL || socket->type != LUAIO_TYPE_SOCKET) { \
    return luaL_argerror(L, 1, "socket:"#name" error: socket must be [userdata](socket)\n"); \
  }

static LuaIO_pool_t LuaIO_tcp_connect_req_pool;
static LuaIO_pool_t LuaIO_tcp_write_req_pool;
static LuaIO_pool_t LuaIO_tcp_shutdown_req_pool;

void LuaIO_tcp_connect_req_pool_init(size_t max_free_chunks) {
  LuaIO_pool_init(&LuaIO_tcp_connect_req_pool, max_free_chunks);
}

void LuaIO_tcp_write_req_pool_init(size_t max_free_chunks) {
  LuaIO_pool_init(&LuaIO_tcp_write_req_pool, max_free_chunks);
}

void LuaIO_tcp_shutdown_req_pool_init(size_t max_free_chunks) {
  LuaIO_pool_init(&LuaIO_tcp_shutdown_req_pool, max_free_chunks);
}

/*local socket, err = tcp.new()*/
static int LuaIO_tcp_socket_new(lua_State *L) {
  LuaIO_tcp_socket_t *socket = lua_newuserdata(L, sizeof(LuaIO_tcp_socket_t));
  if (socket == NULL) {
    lua_pushnil(L);
    lua_pushinteger(L, UV_ENOMEM);
    return 2;
  }

  uv_loop_t *loop = uv_default_loop();
  uv_tcp_init(loop, &socket->handle);

  socket->type = LUAIO_TYPE_SOCKET;
  socket->thread = L;
  socket->current_thread = L;
  socket->read_buffer = NULL;
  socket->timer = NULL;
  socket->timeout = 0;
  socket->onconnect_ref = LUA_NOREF;
  socket->thread_ref = LUA_NOREF;
  socket->write_callback_ref = LUA_NOREF;

  lua_pushlightuserdata(L, &LuaIO_tcp_socket_metatable_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_setmetatable(L, -2);

  lua_pushinteger(L, 0);
  return 2;
}

#define LuaIO_tcp_check_port_and_host(L, name) \
  int port = luaL_checkinteger(L, 2); \
  if (port < 0 || port > 65535) { \
    return luaL_argerror(L, 2, "socket:"#name" error: port must be [0, 65535]\n"); \
  } \
  \
  struct sockaddr *addr; \
  struct sockaddr_in addr4; \
  struct sockaddr_in6 addr6; \
  const char *host = luaL_checkstring(L, 3); \
  if (uv_ip4_addr(host, port, &addr4) == 0) { \
    addr = (struct sockaddr*)(&addr4); \
  } else if (uv_ip6_addr(host, port, &addr6) == 0) { \
    addr = (struct sockaddr*)(&addr6); \
  } else { \
    return luaL_argerror(L, 3, "socket:"#name" error: host is not a IP address\n"); \
  }

/*local err = socket:bind(port, host, tcp_reuseport)*/
static int LuaIO_tcp_socket_bind(lua_State *L) {
  LuaIO_tcp_check_socket(L, bind(port, host, tcp_reuseport));
  LuaIO_tcp_check_port_and_host(L, bind(port, host, tcp_reuseport));

  /*tcp_reuseport*/
  int tcp_reuseport = lua_toboolean(L, 4);
  /*uv.h +521 src/unix/tcp.c +63 +81 +82*/
  int err = uv_tcp_bind(&socket->handle, addr, 0, tcp_reuseport);

  lua_pushinteger(L, err);
  return 1;
}

static void LuaIO_tcp_server_onconnect(uv_stream_t *handle, int status) {
  if (status ) {
    fprintf(stderr, "server onconnect error: %s\n", uv_strerror(status));
    return;
  }

  LuaIO_tcp_socket_t* server = container_of(handle, LuaIO_tcp_socket_t, handle);
  lua_State *L = server->thread;
  lua_State *co = lua_newthread(L);
  int thread_ref = luaL_ref(L, LUA_REGISTRYINDEX);

  /*onconnect*/
  lua_rawgeti(co, LUA_REGISTRYINDEX, server->onconnect_ref);

  LuaIO_tcp_socket_t *socket = lua_newuserdata(co, sizeof(LuaIO_tcp_socket_t));
  if (socket == NULL) {
    luaL_unref(L, LUA_REGISTRYINDEX, thread_ref);
    fprintf(stderr, "server onconnect error: no memory for new connection\n");
    return;
  }

  uv_loop_t *loop = uv_default_loop();
  uv_tcp_t *client_handle = &socket->handle;
  uv_tcp_init(loop, client_handle);
  int err = uv_accept(handle, (uv_stream_t*)client_handle);
  if (err) {
    luaL_unref(L, LUA_REGISTRYINDEX, thread_ref);
    uv_close((uv_handle_t*)(client_handle), NULL);
    fprintf(stderr, "server onconnect error: %s\n", uv_strerror(err));
    return;
  }

  socket->type = LUAIO_TYPE_SOCKET;
  socket->thread = co;
  socket->current_thread = co;
  socket->read_buffer = NULL;
  socket->timer = NULL;
  socket->timeout = server->timeout;
  socket->onconnect_ref = LUA_NOREF;
  socket->thread_ref = thread_ref;
  socket->write_callback_ref = LUA_NOREF;

  lua_pushlightuserdata(co, &LuaIO_tcp_socket_metatable_key);
  lua_rawget(co, LUA_REGISTRYINDEX);
  lua_setmetatable(co, -2);

  LuaIO_resume(co, 1);
}

/*local err = socket:listen(onconnect, tcp_backlog)*/
static int LuaIO_tcp_socket_listen(lua_State *L) {
  LuaIO_tcp_check_socket(L, listen(onconnect, tcp_backlog));

  /*onconnect*/
  if (lua_type(L, 2) != LUA_TFUNCTION) {
    return luaL_argerror(L, 2, "socket:listen(onconnect, tcp_backlog) error: onconnect must be [function]\n"); 
  }
  lua_pushvalue(L, 2);
  socket->onconnect_ref = luaL_ref(L, LUA_REGISTRYINDEX);

  int tcp_backlog = luaL_checkinteger(L, 3);
  int err = uv_listen((uv_stream_t*)(&socket->handle), tcp_backlog, LuaIO_tcp_server_onconnect);

  lua_pushinteger(L, err);
  return 1;
}

static void LuaIO_tcp_socket_connect_timeout(uv_timer_t *handle) {
  LuaIO_tcp_connect_req_t *LuaIO_req = container_of(handle, LuaIO_tcp_connect_req_t, timer);
  lua_State *L = LuaIO_req->current_thread;

  LuaIO_timer_free(handle);

  lua_pushinteger(L, UV_ETIMEDOUT);
  LuaIO_resume(L, 1);
}

static void LuaIO_tcp_socket_onconnect(uv_connect_t *req, int status) {
  LuaIO_tcp_connect_req_t *LuaIO_req = container_of(req, LuaIO_tcp_connect_req_t, req);
  lua_State *L = LuaIO_req->current_thread;

  uv_timer_t *timer = LuaIO_req->timer;
  if (timer != NULL) {
    uv_timer_stop(timer);
    LuaIO_timer_free(timer);
  }

  lua_pushinteger(L, status);
  LuaIO_resume(L, 1);
}

/*local err = socket:connect(port, host)*/
static int LuaIO_tcp_socket_connect(lua_State *L) {
  LuaIO_tcp_check_socket(L, connect(port, host));
  LuaIO_tcp_check_port_and_host(L, connect(port, host));

  uint64_t timeout = socket->timeout;
  uv_timer_t *timer = NULL;
  if (timeout != 0) {
    timer = LuaIO_timer_alloc();
    if (timer == NULL) {
      lua_pushinteger(L, UV_ENOMEM);
      return 1;
    }

    uv_timer_start(timer, 
                   LuaIO_tcp_socket_connect_timeout, 
                   timeout, 
                   0);
  }


  LuaIO_tcp_connect_req_t *LuaIO_req = LuaIO_palloc(&LuaIO_tcp_connect_req_pool,
                                                    sizeof(LuaIO_tcp_connect_req_t));
  if (LuaIO_req == NULL) {
    if (timer != NULL) {
      uv_timer_stop(timer);
      LuaIO_timer_free(timer);
    }
    
    lua_pushinteger(L, UV_ENOMEM);
    return 1;
  }

  int err = uv_tcp_connect(&LuaIO_req->req, 
                           &socket->handle,
                           addr,
                           LuaIO_tcp_socket_onconnect);
  if (err) {
    if (timer != NULL) {
      uv_timer_stop(timer);
      LuaIO_timer_free(timer);
    }
    
    lua_pushinteger(L, err);
    return 1;
  }

  LuaIO_req->current_thread = L;
  LuaIO_req->timer = timer;
  return lua_yield(L, 0);
}

/*local fd = socket:fd()*/
static int LuaIO_tcp_socket_fd(lua_State *L) {
  LuaIO_tcp_check_socket(L, fd());

  /*uv.h +[74-71] src/unix/internal.h -[244-249]*/
  lua_pushinteger(L, uv__stream_fd(&socket->handle));
  return 1;
}

/*socket:set_read_buffer(buffer)*/
static int LuaIO_tcp_socket_set_read_buffer(lua_State *L) {
  LuaIO_tcp_check_socket(L, read(buffer));

  LuaIO_buffer_t *buffer = lua_touserdata(L, 2);
  if (buffer == NULL || buffer->type != LUAIO_TYPE_READ_BUFFER) {
    return luaL_argerror(L, 2, "socket:setReadBuffer(buffer) error: buffer must be [ReadBuffer]\n");
  }

  socket->read_buffer = buffer;
  return 0;
}

static void LuaIO_tcp_socket_read_timeout(uv_timer_t *handle) {
  LuaIO_tcp_socket_t *socket = container_of(handle, LuaIO_tcp_socket_t, timer);
  lua_State *L = socket->current_thread;

  uv_read_stop((uv_stream_t*)(&socket->handle));
  LuaIO_timer_free(handle);

  lua_pushinteger(L, UV_ETIMEDOUT);
  LuaIO_resume(L, 1);
}

static void LuaIO_tcp_socket_onalloc(uv_handle_t *handle, 
                                     size_t suggested_size, 
                                     uv_buf_t *buf) {
  LuaIO_tcp_socket_t *socket = container_of(handle, LuaIO_tcp_socket_t, handle);
  lua_State* L = socket->current_thread;

  LuaIO_buffer_t *buffer = socket->read_buffer;
  if (buffer->capacity == 0) {
    size_t capacity;
    char *start = LuaIO_pmemory__alloc(buffer->size, &capacity);
    if (start == NULL) {
      uv_read_stop((uv_stream_t*)(&socket->handle));

      uv_timer_t *timer = socket->timer;
      if (timer != NULL) {
        uv_timer_stop(timer);
        LuaIO_timer_free(timer);
      }

      lua_pushinteger(L, UV_ENOMEM);
      LuaIO_resume(L, 1);
      return;
    }

    buffer->capacity = capacity;
    buffer->start = start;
    buffer->read_pos = start;
    /*buffer->parse_pos = start;*/
    buffer->write_pos = start;
    buffer->end = start + capacity;
  }

  char *write_pos = buffer->write_pos;
  buf->base = write_pos;
  buf->len = buffer->end - write_pos;
}

static void LuaIO_tcp_socket_onread(uv_stream_t *handle, 
                                    ssize_t nread, 
                                    const uv_buf_t* buf) {
  if (nread == 0) return;
  
  LuaIO_tcp_socket_t *socket = container_of(handle, LuaIO_tcp_socket_t, handle);
  lua_State* L = socket->current_thread;

  uv_read_stop((uv_stream_t*)(&socket->handle));

  uv_timer_t *timer = socket->timer;
  if (timer != NULL) {
    uv_timer_stop(timer);
    LuaIO_timer_free(timer);
  }

  if (nread > 0) {
    socket->read_buffer->write_pos += nread;
  }

  lua_pushinteger(L, nread);
  LuaIO_resume(L, 1);
}

/*local ret = socket:read()*/
static int LuaIO_tcp_socket_read(lua_State *L) {
  LuaIO_tcp_check_socket(L, read(buffer));

  uint64_t timeout = socket->timeout;
  uv_timer_t *timer = NULL;
  if (timeout != 0) {
    timer = LuaIO_timer_alloc();
    if (timer == NULL) {
      lua_pushinteger(L, UV_ENOMEM);
      return 1;
    }

    uv_timer_start(timer, 
                   LuaIO_tcp_socket_read_timeout, 
                   timeout, 
                   0);
  }

  int err = uv_read_start((uv_stream_t*)(&socket->handle), 
                LuaIO_tcp_socket_onalloc, 
                LuaIO_tcp_socket_onread);
  if (err) {
    if (timer != NULL) {
      uv_timer_stop(timer);
      LuaIO_timer_free(timer);
    }
    
    lua_pushinteger(L, err);
    return 1;
  }

  socket->timer = timer;
  socket->current_thread = L;
  return lua_yield(L, 0);
}

/*socket:set_write_callback(callback)*/
static int LuaIO_tcp_socket_set_write_callback(lua_State *L) {
  LuaIO_tcp_check_socket(L, read(buffer));

  if (lua_type(L, 2) != LUA_TFUNCTION) {
    return luaL_argerror(L, 2, "socket:setWriteCallback(callback) error: callback must be [function]\n"); 
  }
  lua_pushvalue(L, 2);
  socket->write_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  
  return 0;
}

static int LuaIO_tcp_socket_try_write(uv_stream_t *handle, 
                                      uv_buf_t **bufs, 
                                      size_t *count, 
                                      size_t *written_bytes) {
  uv_buf_t *vbufs = *bufs;
  size_t vcount = *count;

  int err = uv_try_write(handle, vbufs, vcount);
  if (err == UV_ENOSYS || err == UV_EAGAIN) {
    return 0;
  }

  if (err < 0) {
    return err;
  }

  *written_bytes = err;
  size_t written = err;
  for (; written != 0 && vcount > 0; vbufs++, vcount--) {
    if (vbufs[0].len > written) {
      vbufs[0].base += written;
      vbufs[0].len -= written;
      written = 0;
      break;
    } else {
      written -= vbufs[0].len;
    }
  }

  *bufs = vbufs;
  *count = vcount;

  return 0;
}

static void LuaIO_tcp_socket_write_timeout(uv_timer_t *handle) {
  LuaIO_tcp_write_req_t *LuaIO_req = container_of(handle, LuaIO_tcp_write_req_t, timer);
  lua_State *L = LuaIO_req->current_thread;
  
  LuaIO_timer_free(handle);
  luaL_unref(L, LUA_REGISTRYINDEX, LuaIO_req->write_data_ref);

  int write_callback_ref = LuaIO_req->write_callback_ref;
  if (write_callback_ref != LUA_NOREF) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, LuaIO_req->write_callback_ref);
    lua_pushinteger(L, UV_ETIMEDOUT);
    LuaIO_pcall(L, 1);
  }
}

static void LuaIO_tcp_socket_after_write(uv_write_t *req, int status) {
  LuaIO_tcp_write_req_t *LuaIO_req = container_of(req, LuaIO_tcp_write_req_t, req);
  lua_State* L = LuaIO_req->current_thread;
 
  uv_timer_t *timer = LuaIO_req->timer;
  if (timer != NULL) {
    uv_timer_stop(timer);
    LuaIO_timer_free(timer);
  }

  luaL_unref(L, LUA_REGISTRYINDEX, LuaIO_req->write_data_ref);

  int write_callback_ref = LuaIO_req->write_callback_ref;
  if (write_callback_ref != LUA_NOREF) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, write_callback_ref);
    lua_pushinteger(L, status);
    LuaIO_pcall(L, 1);
  }
}

/*local bytes, err = socket:write(data)*/
static int LuaIO_tcp_socket_write(lua_State *L) {
  LuaIO_tcp_check_socket(L, write(data));
  /*common.h*/
  LuaIO_check_data(L, 2, socket:write(data));

  size_t written = 0;
  size_t vcount = count;
  uv_stream_t *stream_handle = (uv_stream_t*)(&socket->handle);
  int err = LuaIO_tcp_socket_try_write(stream_handle, 
                                       &bufs, 
                                       &vcount, 
                                       &written);
  if (err) {
    if (tmp != NULL) LuaIO_pmemory_free(tmp);
    lua_pushinteger(L, 0);
    lua_pushinteger(L, err);
    return 2;
  }

  if (vcount == 0) {
    if (tmp != NULL) LuaIO_pmemory_free(tmp);
    lua_pushinteger(L, written);
    /*uv_try_write send all data*/
    lua_pushinteger(L, 1);
    return 2;
  }

  uint64_t timeout = socket->timeout;
  uv_timer_t *timer = NULL;
  if (timeout != 0) {
    timer = LuaIO_timer_alloc();
    if (timer == NULL) {
      if (tmp != NULL) LuaIO_pmemory_free(tmp);
      lua_pushinteger(L, written);
      lua_pushinteger(L, UV_ENOMEM);
      return 2;
    }

    uv_timer_start(timer, 
                   LuaIO_tcp_socket_write_timeout, 
                   timeout, 
                   0);
  }

  LuaIO_tcp_write_req_t *LuaIO_req = LuaIO_palloc(&LuaIO_tcp_write_req_pool,
                                                  sizeof(LuaIO_tcp_write_req_t));
  if (LuaIO_req == NULL) {
    if (tmp != NULL) LuaIO_pmemory_free(tmp);
    if (timer != NULL) {
      uv_timer_stop(timer);
      LuaIO_timer_free(timer);
    }

    lua_pushinteger(L, written);
    lua_pushinteger(L, UV_ENOMEM);
    return 2;
  }

  err = uv_write2(&LuaIO_req->req, 
                  stream_handle, 
                  bufs, 
                  vcount, 
                  NULL, 
                  LuaIO_tcp_socket_after_write);
  if (err) {
    if (tmp != NULL) LuaIO_pmemory_free(tmp);
    if (timer != NULL) {
      uv_timer_stop(timer);
      LuaIO_timer_free(timer);
    }

    lua_pushinteger(L, written);
    lua_pushinteger(L, err);
    return 2;
  }

  lua_pushvalue(L, 2);
  LuaIO_req->write_data_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  LuaIO_req->current_thread = L;
  LuaIO_req->write_callback_ref = socket->write_callback_ref;
  LuaIO_req->timer = timer;

  if (tmp != NULL) LuaIO_pmemory_free(tmp);

  lua_pushinteger(L, bytes);
  lua_pushinteger(L, err);
  return 2;
}

/*local addr, err = socket:local_address()*/
static int LuaIO_tcp_socket_local_address(lua_State *L) {
  LuaIO_tcp_check_socket(L, localAddress());

  struct sockaddr_storage address;
  int len = sizeof(address);
  int ret = uv_tcp_getsockname(&socket->handle, 
                               (struct sockaddr*)&address, &len);
  if (ret == 0) {
    ret = LuaIO_parse_socket_address(L, &address);
  } else {
    lua_pushnil(L);
  }

  lua_pushinteger(L, ret);
  return 2;
}

/*local addr, err = socket:remote_address()*/
static int LuaIO_tcp_socket_remote_address(lua_State *L) {
  LuaIO_tcp_check_socket(L, remoteAddress());

  struct sockaddr_storage address;
  int len = sizeof(address);
  int ret = uv_tcp_getpeername(&socket->handle, 
                               (struct sockaddr*)&address, &len);
  if (ret == 0) {
    ret = LuaIO_parse_socket_address(L, &address);
  } else {
    lua_pushnil(L);
  }

  lua_pushinteger(L, ret);
  return 2;
}

/*socket:set_timeout(timeout)*/
static int LuaIO_tcp_socket_set_timeout(lua_State *L) {
  LuaIO_tcp_check_socket(L, setTimeout(timeout));

  lua_Integer timeout = luaL_checkinteger(L, 2);
  if (timeout < 0) {
    return luaL_argerror(L, 1, "socket:setTimeout(timeout) error: timeout must be >= 0\n");
  }
  socket->timeout = timeout;

  return 0;
}

/*local err = socket:set_nodelay(enable)*/
static int LuaIO_tcp_socket_set_nodelay(lua_State *L) {
  LuaIO_tcp_check_socket(L, setNodelay(enable));

  int enable = lua_toboolean(L, 2);
  int err = uv_tcp_nodelay(&socket->handle, enable);

  lua_pushinteger(L, err);
  return 1;
}

/*local err = socket:set_keepalive(enable, delay)*/
static int LuaIO_tcp_socket_set_keepalive(lua_State *L) {
  LuaIO_tcp_check_socket(L, setKeepalive(enable, delay));

  int delay = 0;
  int enable = lua_toboolean(L, 2);
  if (enable) {
    delay = luaL_checkinteger(L, 3);
  }
  int err = uv_tcp_keepalive(&socket->handle, enable, delay);

  lua_pushinteger(L, err);
  return 1;
}

static void LuaIO_tcp_socket_after_shutdown(uv_shutdown_t *req, int status) {
  lua_State *L = req->data;
  lua_pushinteger(L, status);
  LuaIO_resume(L, 1);
}

/*local err = socket:shutdown()*/
static int LuaIO_tcp_socket_shutdown(lua_State *L) {
  LuaIO_tcp_check_socket(L, shutdown());

  uv_shutdown_t *req = LuaIO_palloc(&LuaIO_tcp_shutdown_req_pool, 
                                    sizeof(uv_shutdown_t));
  if (req == NULL) {
    lua_pushinteger(L, UV_ENOMEM);
    return 1;
  }

  req->data = L;
  int err = uv_shutdown(req, 
                        (uv_stream_t*)(&socket->handle), 
                        LuaIO_tcp_socket_after_shutdown);
  if (err) {
    lua_pushinteger(L, err);
    return 1;
  }

  return lua_yield(L, 0);
}

static void LuaIO_tcp_socket_onclose(uv_handle_t *handle) {
  LuaIO_tcp_socket_t *socket = container_of(handle, LuaIO_tcp_socket_t, handle);
  lua_State *L = socket->thread;

  int onconnect_ref = socket->onconnect_ref;
  if (onconnect_ref != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, onconnect_ref);
    socket->onconnect_ref = LUA_NOREF;
  }

  int write_callback_ref = socket->write_callback_ref;
  if (write_callback_ref != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, write_callback_ref);
    socket->write_callback_ref = LUA_NOREF;
  }

  int thread_ref = socket->thread_ref;
  if (thread_ref != LUA_NOREF) {
    luaL_unref(socket->thread, LUA_REGISTRYINDEX, thread_ref);
    socket->thread = NULL;
    socket->thread_ref = LUA_NOREF;
  }
}

/*socket:close()*/
static int LuaIO_tcp_socket_close(lua_State *L) {
  LuaIO_tcp_check_socket(L, close());

  uv_handle_t *handle = (uv_handle_t*)(&socket->handle);
  if (uv_is_closing(handle)) {
    luaL_error(L, "socket:close() error: socket is already closing");
  }

  uv_close(handle, LuaIO_tcp_socket_onclose);
  return 0;
}

/*tcp.is_ip(string)*/
static int LuaIO_tcp_is_ip(lua_State *L) {
  const char *ip = luaL_checkstring(L, 1);
  char addr[sizeof(struct in6_addr)];
  
  int rc = 0;
  if (uv_inet_pton(AF_INET, ip, &addr) == 0) {
    rc = 4;
  } else if (uv_inet_pton(AF_INET6, ip, &addr) == 0) {
    rc = 6;
  }

  lua_pushinteger(L, rc);
  return 1;
}

int luaopen_tcp(lua_State *L) {
  /*tcp metatable*/
  lua_pushlightuserdata(L, &LuaIO_tcp_socket_metatable_key);

  lua_createtable(L, 0, 16);
  LuaIO_function(LuaIO_tcp_socket_bind, "bind")
  LuaIO_function(LuaIO_tcp_socket_listen, "listen")
  LuaIO_function(LuaIO_tcp_socket_connect, "connect")
  LuaIO_function(LuaIO_tcp_socket_fd, "fd")
  LuaIO_function(LuaIO_tcp_socket_set_read_buffer, "set_read_buffer")
  LuaIO_function(LuaIO_tcp_socket_read, "read")
  LuaIO_function(LuaIO_tcp_socket_set_write_callback, "set_write_callback")
  LuaIO_function(LuaIO_tcp_socket_write, "write")
  LuaIO_function(LuaIO_tcp_socket_local_address, "local_address")
  LuaIO_function(LuaIO_tcp_socket_remote_address, "remote_address")
  LuaIO_function(LuaIO_tcp_socket_set_timeout, "set_timeout")
  LuaIO_function(LuaIO_tcp_socket_set_nodelay, "set_nodelay")
  LuaIO_function(LuaIO_tcp_socket_set_keepalive, "set_keepalive")
  LuaIO_function(LuaIO_tcp_socket_shutdown, "shutdown")
  LuaIO_function(LuaIO_tcp_socket_close, "close")

  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  /*save tcp metatable*/
  lua_rawset(L, LUA_REGISTRYINDEX);

  luaL_Reg lib[] = {
    { "new", LuaIO_tcp_socket_new },
    { "is_ip", LuaIO_tcp_is_ip },
    { "__newindex", LuaIO_cannot_change },
    { NULL, NULL }
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
