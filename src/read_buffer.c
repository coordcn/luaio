/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "buffer.h"

static char LuaIO_read_buffer_metatable_key;

#define LuaIO_buffer_check_read_buffer(L, name) \
  LuaIO_buffer_t *buffer = lua_touserdata(L, 1); \
  if (buffer == NULL || buffer->type != LUAIO_TYPE_READ_BUFFER) { \
    return luaL_argerror(L, 1, "buffer:"#name" error: buffer must be [userdata](read_buffer)\n"); \
  }

#define LuaIO_buffer_check_rest_size(n) \
  if (n == rest_size) { \
    start = buffer->start; \
    buffer->read_pos = start; \
    buffer->write_pos = start; \
  } else { \
    buffer->read_pos += n; \
  }

/* local read_buffer = require('read_buffer')
 * local buffer = read_buffer.new(size)
 */
static int LuaIO_read_buffer_new(lua_State *L) {
  lua_Integer size = luaL_checkinteger(L, 1);
  if (size <= 0) {
    return luaL_argerror(L, 1, "read_buffer.new(size) error: size must be > 0\n"); 
  }

  LuaIO_buffer_t *buffer = lua_newuserdata(L, sizeof(LuaIO_buffer_t));
  if (buffer == NULL) {
    lua_pushnil(L);
    return 1;
  }

  buffer->type = LUAIO_TYPE_READ_BUFFER;
  buffer->size = size;
  buffer->capacity = 0;
  buffer->start = NULL;
  buffer->read_pos = NULL;
  buffer->write_pos = NULL;
  buffer->end = NULL;

  /*read buffer metatable*/
  lua_pushlightuserdata(L, &LuaIO_read_buffer_metatable_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_setmetatable(L, -2);
  return 1;
}

/* local data, err = buffer:read(n)  read n bytes data
 * local rest, err = buffer:read()   read rest data
 */
static int LuaIO_buffer_read(lua_State *L) {
  LuaIO_buffer_check_read_buffer(L, read([n]));
  LuaIO_buffer_check_memory(L, read([n]));

  lua_Integer n = luaL_checkinteger(L, 2);

  int rest_size;
  char *read_pos;
  char *start;
  if (n < 0) {
    read_pos = buffer->read_pos;
    start = buffer->start;
    rest_size = buffer->write_pos - read_pos;
    assert(rest_size >= 0);

    if (rest_size > 0) {
      lua_pushlstring(L, read_pos, rest_size);
      lua_pushinteger(L, rest_size);

      start = buffer->start;
      buffer->read_pos = start;
      buffer->write_pos = start;
    } else if (rest_size == 0) {
      lua_pushliteral(L, "");
      lua_pushinteger(L, 0);
    }

    return 2;
  }

  if (n == 0) {
    lua_pushliteral(L, "");
    lua_pushinteger(L, 0);
    return 2;
  }

  if (n > (lua_Integer)buffer->capacity) {
    return luaL_error(L, "buffer:read([n]) error: out of buffer capacity[%d]\n", buffer->capacity); 
  }

  read_pos = buffer->read_pos;
  rest_size = buffer->write_pos - read_pos;
  assert(rest_size >= 0);
  /* start   read_pos   n   write_pos   end 
   * |          |       |      |          |
   * -----------+++++++++++++++------------
   */
  if (n <= rest_size) {
    lua_pushlstring(L, read_pos, n);
    lua_pushinteger(L, n);
    
    LuaIO_buffer_check_rest_size(n);
    return 2;
  }

  /* start   read_pos   write_pos   end   n
   * |          |          |         |    |
   * -----------++++++++++++---------------
   */
  if ((read_pos + n) > buffer->end) {
    start = buffer->start;
    LuaIO_memmove(start, read_pos, rest_size);
    buffer->read_pos = start;
    buffer->write_pos = start + rest_size;
  }

  lua_pushnil(L);
  lua_pushinteger(L, LUAIO_EAGAIN);
  return 2;
}

/* local data, err = buf:readline() */
static int LuaIO_buffer_readline(lua_State *L) {
  LuaIO_buffer_check_read_buffer(L, readline());
  LuaIO_buffer_check_memory(L, readline());

  char *read_pos = buffer->read_pos;
  char *write_pos = buffer->write_pos;
  char *pos = read_pos;
  char *start;
  int flag = 0;
  int rest_size;
  char c;

  while (pos < write_pos) {
    c = *pos;
    pos++;
    if (c == '\n') {
      flag = 1;
      break;
    }
  }

  /* start   read_pos   \n   write_pos   end 
   * |          |        |      |          |
   * -----------+++++++++++++++++-----------
   */
  if (flag) {
    rest_size = buffer->write_pos - buffer->read_pos;
    assert(rest_size >= 0);

    int n = pos - read_pos;
    lua_pushlstring(L, buffer->read_pos, n);
    lua_pushinteger(L, n);

    LuaIO_buffer_check_rest_size(n);
    return 2;
  }

  if (write_pos == buffer->end) {
    start = buffer->start;

    /* start == read_pos   write_pos == end
     * |                           |
     * +++++++++++++++++++++++++++++
     */
    if (read_pos == start) {
      lua_pushnil(L);
      lua_pushinteger(L, LUAIO_EXCEED_BUFFER_CAPACITY);
      return 2;
    }

    /* start  read_pos   write_pos == end
     * |        |                   |
     * ---------+++++++++++++++++++++
     */
    rest_size = write_pos - read_pos;
    assert(rest_size >= 0);

    LuaIO_memmove(start, read_pos, rest_size);
    buffer->read_pos = start;
    buffer->write_pos = start + rest_size;
  }

  lua_pushnil(L);
  lua_pushinteger(L, LUAIO_EAGAIN);
  return 2;
}

#define LuaIO_buffer_read8(type, bytes) do{ \
  LuaIO_buffer_check_read_buffer(L, read_##type()); \
  LuaIO_buffer_check_memory(L, read_##type()); \
  \
  char *start; \
  char *read_pos = buffer->read_pos; \
  int rest_size = buffer->write_pos - read_pos; \
  assert(rest_size >= 0); \
  \
  if (bytes <= (size_t)rest_size) { \
    type temp = *((type*)read_pos); \
    lua_pushinteger(L, temp); \
    lua_pushinteger(L, bytes); \
    \
    LuaIO_buffer_check_rest_size(bytes); \
    return 2; \
  } \
  \
  lua_pushnil(L); \
  lua_pushinteger(L, LUAIO_EAGAIN); \
  return 2; \
} while (0)

/* local data, err = buf:read_uint8() */
static int LuaIO_buffer_read_uint8(lua_State *L) {
  LuaIO_buffer_read8(uint8_t, sizeof(uint8_t));
}

/* local data, err = buf:read_int8() */
static int LuaIO_buffer_read_int8(lua_State *L) {
  LuaIO_buffer_read8(int8_t, sizeof(int8_t));
}

#define LuaIO_buffer_read_uint(name, type, bytes, endian) do { \
  LuaIO_buffer_check_read_buffer(L, read_##name()); \
  LuaIO_buffer_check_memory(L, read_##name()); \
  \
  char *start; \
  char *read_pos = buffer->read_pos; \
  int rest_size = buffer->write_pos - read_pos; \
  assert(rest_size >= 0); \
  \
  if (bytes <= (size_t)rest_size) { \
    type temp = *((type*)read_pos); \
    temp = endian(temp); \
    lua_pushinteger(L, temp); \
    lua_pushinteger(L, bytes); \
    \
    LuaIO_buffer_check_rest_size(bytes); \
    return 2; \
  } \
  \
  if ((read_pos + bytes) > buffer->end) { \
    start = buffer->start; \
    LuaIO_memmove(start, read_pos, rest_size); \
    buffer->read_pos = start; \
    buffer->write_pos = start + rest_size; \
  } \
  \
  lua_pushnil(L); \
  lua_pushinteger(L, LUAIO_EAGAIN); \
  return 2; \
} while (0)

static int LuaIO_buffer_read_uint16_le(lua_State *L) {
  LuaIO_buffer_read_uint(uint16_le, uint16_t, sizeof(uint16_t), le16toh);
}

static int LuaIO_buffer_read_uint16_be(lua_State *L) {
  LuaIO_buffer_read_uint(uint16_be, uint16_t, sizeof(uint16_t), be16toh);
}

static int LuaIO_buffer_read_uint32_le(lua_State *L) {
  LuaIO_buffer_read_uint(uint32_le, uint32_t, sizeof(uint32_t), le32toh);
}

static int LuaIO_buffer_read_uint32_be(lua_State *L) {
  LuaIO_buffer_read_uint(uint32_be, uint32_t, sizeof(uint32_t), be32toh);
}

static int LuaIO_buffer_read_uint64_le(lua_State *L) {
  LuaIO_buffer_read_uint(uint64_le, uint64_t, sizeof(uint64_t), le64toh);
}

static int LuaIO_buffer_read_uint64_be(lua_State *L) {
  LuaIO_buffer_read_uint(uint64_be, uint64_t, sizeof(uint64_t), be64toh);
}

#define LuaIO_buffer_read_int(name, type, bytes, endian) do { \
  LuaIO_buffer_check_read_buffer(L, read_##name()); \
  LuaIO_buffer_check_memory(L, read_##name()); \
  \
  char *start; \
  char *read_pos = buffer->read_pos; \
  int rest_size = buffer->write_pos - read_pos; \
  assert(rest_size >= 0); \
  \
  if (bytes <= (size_t)rest_size) { \
    u##type temp = *((u##type*)read_pos); \
    temp = endian(temp); \
    type tmp = (type)temp; \
    lua_pushinteger(L, tmp); \
    lua_pushinteger(L, bytes); \
    \
    LuaIO_buffer_check_rest_size(bytes); \
    return 2; \
  } \
  \
  if ((read_pos + bytes) > buffer->end) { \
    start = buffer->start; \
    LuaIO_memmove(start, read_pos, rest_size); \
    buffer->read_pos = start; \
    buffer->write_pos = start + rest_size; \
  } \
  \
  lua_pushnil(L); \
  lua_pushinteger(L, LUAIO_EAGAIN); \
  return 2; \
} while (0)

static int LuaIO_buffer_read_int16_le(lua_State *L) {
  LuaIO_buffer_read_uint(int16_le, int16_t, sizeof(int16_t), le16toh);
}

static int LuaIO_buffer_read_int16_be(lua_State *L) {
  LuaIO_buffer_read_uint(int16_be, int16_t, sizeof(int16_t), be16toh);
}

static int LuaIO_buffer_read_int32_le(lua_State *L) {
  LuaIO_buffer_read_uint(int32_le, int32_t, sizeof(int32_t), le32toh);
}

static int LuaIO_buffer_read_int32_be(lua_State *L) {
  LuaIO_buffer_read_uint(int32_be, int32_t, sizeof(int32_t), be32toh);
}

static int LuaIO_buffer_read_int64_le(lua_State *L) {
  LuaIO_buffer_read_uint(int64_le, int64_t, sizeof(int64_t), le64toh);
}

static int LuaIO_buffer_read_int64_be(lua_State *L) {
  LuaIO_buffer_read_uint(int64_be, int64_t, sizeof(int64_t), be64toh);
}

#define LuaIO_buffer_read_float(name, type, bytes, endian, temp_type) do { \
  LuaIO_buffer_check_read_buffer(L, read_##name()); \
  LuaIO_buffer_check_memory(L, read_##name()); \
  \
  char *start; \
  char *read_pos = buffer->read_pos; \
  int rest_size = buffer->write_pos - read_pos; \
  assert(rest_size >= 0); \
  \
  if (bytes <= (size_t)rest_size) { \
    temp_type *temp = (temp_type*)read_pos; \
    temp_type tmp = endian(*temp); \
    type *t = (type*)&tmp; \
    lua_pushnumber(L, *t); \
    lua_pushinteger(L, bytes); \
    \
    LuaIO_buffer_check_rest_size(bytes); \
    return 2; \
  } \
  \
  if ((read_pos + bytes) > buffer->end) { \
    start = buffer->start; \
    LuaIO_memmove(start, read_pos, rest_size); \
    buffer->read_pos = start; \
    buffer->write_pos = start + rest_size; \
  } \
  \
  lua_pushnil(L); \
  lua_pushinteger(L, LUAIO_EAGAIN); \
  return 2; \
} while (0)

static int LuaIO_buffer_read_float_le(lua_State *L) {
  LuaIO_buffer_read_float(float_le, float, sizeof(float), le32toh, uint32_t);
}

static int LuaIO_buffer_read_float_be(lua_State *L) {
  LuaIO_buffer_read_float(float_be, float, sizeof(float), be32toh, uint32_t);
}

static int LuaIO_buffer_read_double_le(lua_State *L) {
  LuaIO_buffer_read_float(double_le, double, sizeof(double), le64toh, uint64_t);
}

static int LuaIO_buffer_read_double_be(lua_State *L) {
  LuaIO_buffer_read_float(double_be, double, sizeof(double), be64toh, uint64_t);
}

int luaopen_read_buffer(lua_State *L) {
  /*read buffer metatable*/
  lua_pushlightuserdata(L, &LuaIO_read_buffer_metatable_key);

  lua_createtable(L, 0, 24);
  LuaIO_function(LuaIO_buffer_capacity, "capacity")
  LuaIO_function(LuaIO_buffer_discard, "discard")
  LuaIO_function(LuaIO_buffer_read, "read")
  LuaIO_function(LuaIO_buffer_readline, "readline")
  LuaIO_function(LuaIO_buffer_read_uint8, "read_uint8")
  LuaIO_function(LuaIO_buffer_read_int8, "read_int8")
  LuaIO_function(LuaIO_buffer_read_uint16_le, "read_uint16_le")
  LuaIO_function(LuaIO_buffer_read_uint16_be, "read_uint16_be")
  LuaIO_function(LuaIO_buffer_read_uint32_le, "read_uint32_le")
  LuaIO_function(LuaIO_buffer_read_uint32_be, "read_uint32_be")
  LuaIO_function(LuaIO_buffer_read_uint64_le, "read_uint64_le")
  LuaIO_function(LuaIO_buffer_read_uint64_be, "read_uint64_be")
  LuaIO_function(LuaIO_buffer_read_int16_le, "read_int16_le")
  LuaIO_function(LuaIO_buffer_read_int16_be, "read_int16_be")
  LuaIO_function(LuaIO_buffer_read_int32_le, "read_int32_le")
  LuaIO_function(LuaIO_buffer_read_int32_be, "read_int32_be")
  LuaIO_function(LuaIO_buffer_read_int64_le, "read_int64_le")
  LuaIO_function(LuaIO_buffer_read_int64_be, "read_int64_be")
  LuaIO_function(LuaIO_buffer_read_float_le, "read_float_le")
  LuaIO_function(LuaIO_buffer_read_float_be, "read_float_be")
  LuaIO_function(LuaIO_buffer_read_double_le, "read_double_le")
  LuaIO_function(LuaIO_buffer_read_double_be, "read_double_be")
  LuaIO_function(LuaIO_buffer_gc, "__gc")

  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  /*save read buffer metatable*/
  lua_rawset(L, LUA_REGISTRYINDEX);

  luaL_Reg lib[] = {
    { "new", LuaIO_read_buffer_new },
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
