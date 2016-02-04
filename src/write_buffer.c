/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "buffer.h"

static char LuaIO_write_buffer_metatable_key;

#define LuaIO_buffer_check_write_buffer(L, name) \
  LuaIO_buffer_t *buffer = lua_touserdata(L, 1); \
  if (buffer == NULL || buffer->type != LUAIO_TYPE_WRITE_BUFFER) { \
    return luaL_argerror(L, 1, "buffer:"#name" error: buffer must be [userdata](write_buffer)\n"); \
  }

/* local write_buffer = require('write_buffer')
 * local buffer = write_buffer.new(size)
 */
static int LuaIO_write_buffer_new(lua_State *L) {
  lua_Integer size = luaL_checkinteger(L, 1);
  if (size <= 0) {
    return luaL_argerror(L, 1, "write_buffer.new(size) error: size must be > 0\n"); 
  }

  LuaIO_buffer_t *buffer = lua_newuserdata(L, sizeof(LuaIO_buffer_t));
  if (buffer == NULL) {
    lua_pushnil(L);
    return 1;
  }

  size_t capacity;
  char *start = LuaIO_pmemory__alloc(size, &capacity);
  if (start == NULL) {
    lua_pop(L, 1);
    lua_pushnil(L);
    return 1;
  }

  buffer->type = LUAIO_TYPE_WRITE_BUFFER;
  buffer->size = size;
  buffer->capacity = capacity;
  buffer->start = start;
  buffer->read_pos = start;
  buffer->write_pos = start;
  buffer->end = start + capacity;

  /*buffer metatable*/
  lua_pushlightuserdata(L, &LuaIO_write_buffer_metatable_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_setmetatable(L, -2);
  return 1;
}

/* local err = buffer:write(str) write string */
static int LuaIO_buffer_write(lua_State *L) {
  LuaIO_buffer_check_write_buffer(L, write(str));
  LuaIO_buffer_check_memory(L, write(str));

  size_t len;
  const char *str = luaL_checklstring(L, 2, &len);

  char *write_pos = buffer->write_pos;
  char *read_pos = buffer->read_pos;
  int rest_size = write_pos - read_pos;
  assert(rest_size >= 0);

  char *start;
  char *pos = write_pos + len;
  if (pos <= buffer->end) {
    LuaIO_memcpy(write_pos, str, len);
    buffer->write_pos += len;
    lua_pushinteger(L, len);
  } else if (pos <= (read_pos + buffer->capacity)) {
    start = buffer->start;
    LuaIO_memmove(start, read_pos, rest_size);
    write_pos = start + rest_size;
    LuaIO_memcpy(write_pos, str, len);
    buffer->read_pos = start;
    buffer->write_pos = write_pos + len;
    lua_pushinteger(L, len);
  } else {
    lua_pushinteger(L, LUAIO_EXCEED_BUFFER_CAPACITY);
  }

  return 1;
}

#define LuaIO_buffer_write8(type, bytes, min, max) do { \
  LuaIO_buffer_check_write_buffer(L, write_##type(n)); \
  LuaIO_buffer_check_memory(L, write_##type(n)); \
  \
  lua_Integer n = luaL_checkinteger(L, 2); \
  if (n < min || n > max) { \
    return luaL_error(L, "buffer:write_"#type"(n) error: n out of range[%d, %d]\n", min, max); \
  } \
  \
  char *write_pos = buffer->write_pos; \
  char *read_pos = buffer->read_pos; \
  int rest_size = write_pos - read_pos; \
  assert(rest_size >= 0); \
  \
  char *start; \
  char *pos = write_pos + bytes; \
  if (pos <= buffer->end) { \
    type temp = (type)n; \
    *write_pos = temp; \
    buffer->write_pos += bytes; \
    lua_pushinteger(L, bytes); \
  } else if (pos <= (read_pos + buffer->capacity)) { \
    start = buffer->start; \
    LuaIO_memmove(start, read_pos, rest_size); \
    type temp = (type)n; \
    write_pos = start + rest_size; \
    *write_pos = temp; \
    buffer->read_pos = start; \
    buffer->write_pos = write_pos + bytes; \
    lua_pushinteger(L, bytes); \
  } else { \
    lua_pushinteger(L, LUAIO_EXCEED_BUFFER_CAPACITY); \
  } \
  \
  return 1; \
} while (0)

static int LuaIO_buffer_write_uint8(lua_State *L) {
  LuaIO_buffer_write8(uint8_t, sizeof(uint8_t), 0, UINT8_MAX);
}

static int LuaIO_buffer_write_int8(lua_State *L) {
  LuaIO_buffer_write8(int8_t, sizeof(int8_t), INT8_MIN, INT8_MAX);
}

#define LuaIO_buffer_write_uint(name, type, bytes, endian, min, max) do { \
  LuaIO_buffer_check_write_buffer(L, write_##name(n)); \
  LuaIO_buffer_check_memory(L, write_##name(n)); \
  \
  lua_Integer n = luaL_checkinteger(L, 2); \
  if (n < min || n > max) { \
    return luaL_error(L, "buffer:write_"#name"(n) error: n out of range[%d, %d]\n", min, max); \
  } \
  \
  char *write_pos = buffer->write_pos; \
  char *read_pos = buffer->read_pos; \
  int rest_size = write_pos - read_pos; \
  assert(rest_size >= 0); \
  \
  char *start; \
  char *pos = write_pos + bytes; \
  if (pos <= buffer->end) { \
    type temp = (type)n; \
    temp = endian(temp); \
    *((type*)write_pos) = temp; \
    buffer->write_pos += bytes; \
    lua_pushinteger(L, bytes); \
  } else if (pos <= (read_pos + buffer->capacity)) { \
    start = buffer->start; \
    LuaIO_memmove(start, read_pos, rest_size); \
    type temp = (type)n; \
    temp = endian(temp); \
    write_pos = start + rest_size; \
    *((type*)write_pos) = temp; \
    buffer->read_pos = start; \
    buffer->write_pos = write_pos + bytes; \
    lua_pushinteger(L, bytes); \
  } else { \
    lua_pushinteger(L, LUAIO_EXCEED_BUFFER_CAPACITY); \
  } \
  \
  return 1; \
} while (0)

static int LuaIO_buffer_write_uint16_le(lua_State *L) {
  LuaIO_buffer_write_uint(uint16_le, uint16_t, sizeof(uint16_t), htole16, 0, UINT16_MAX);
}

static int LuaIO_buffer_write_uint16_be(lua_State *L) {
  LuaIO_buffer_write_uint(uint16_be, uint16_t, sizeof(uint16_t), htobe16, 0, UINT16_MAX);
}

static int LuaIO_buffer_write_uint32_le(lua_State *L) {
  LuaIO_buffer_write_uint(uint32_le, uint32_t, sizeof(uint32_t), htole32, 0, UINT32_MAX);
}

static int LuaIO_buffer_write_uint32_be(lua_State *L) {
  LuaIO_buffer_write_uint(uint32_be, uint32_t, sizeof(uint32_t), htobe32, 0, UINT32_MAX);
}

static int LuaIO_buffer_write_uint64_le(lua_State *L) {
  LuaIO_buffer_write_uint(uint64_le, uint64_t, sizeof(uint64_t), htole64, 0, INT64_MAX);
}

static int LuaIO_buffer_write_uint64_be(lua_State *L) {
  LuaIO_buffer_write_uint(uint64_be, uint64_t, sizeof(uint64_t), htobe64, 0, INT64_MAX);
}

#define LuaIO_buffer_write_int(name, type, bytes, endian, min, max) do { \
  LuaIO_buffer_check_write_buffer(L, write_##name(n)); \
  LuaIO_buffer_check_memory(L, write_##name(n)); \
  \
  lua_Integer n = luaL_checkinteger(L, 2); \
  if (n < min || n > max) { \
    return luaL_error(L, "buffer:write_"#name"(n) error: n out of range[%d, %d]\n", min, max); \
  } \
  \
  char *write_pos = buffer->write_pos; \
  char *read_pos = buffer->read_pos; \
  int rest_size = write_pos - read_pos; \
  assert(rest_size >= 0); \
  \
  char *start; \
  char *pos = write_pos + bytes; \
  if (pos <= buffer->end) { \
    type temp = (type)n; \
    u##type tmp = endian((u##type)temp); \
    *((u##type*)write_pos) = tmp; \
    buffer->write_pos += bytes; \
    lua_pushinteger(L, bytes); \
  } else if (pos <= (read_pos + buffer->capacity)) { \
    start = buffer->start; \
    LuaIO_memmove(start, read_pos, rest_size); \
    type temp = (type)n; \
    u##type tmp = endian((u##type)temp); \
    write_pos = start + rest_size; \
    *((u##type*)write_pos) = tmp; \
    buffer->read_pos = start; \
    buffer->write_pos = write_pos + bytes; \
    lua_pushinteger(L, bytes); \
  } else { \
    lua_pushinteger(L, LUAIO_EXCEED_BUFFER_CAPACITY); \
  } \
  \
  return 1; \
} while (0)

static int LuaIO_buffer_write_int16_le(lua_State *L) {
  LuaIO_buffer_write_int(int16_le, int16_t, sizeof(int16_t), htole16, INT16_MIN, INT16_MAX);
}

static int LuaIO_buffer_write_int16_be(lua_State *L) {
  LuaIO_buffer_write_int(int16_be, int16_t, sizeof(int16_t), htobe16, INT16_MIN, INT16_MAX);
}

static int LuaIO_buffer_write_int32_le(lua_State *L) {
  LuaIO_buffer_write_int(int32_le, int32_t, sizeof(int32_t), htole32, INT32_MIN, INT32_MAX);
}

static int LuaIO_buffer_write_int32_be(lua_State *L) {
  LuaIO_buffer_write_int(int32_be, int32_t, sizeof(int32_t), htobe32, INT32_MIN, INT32_MAX);
}

static int LuaIO_buffer_write_int64_le(lua_State *L) {
  LuaIO_buffer_write_int(int64_le, int64_t, sizeof(int64_t), htole64, INT64_MIN, INT64_MAX);
}

static int LuaIO_buffer_write_int64_be(lua_State *L) {
  LuaIO_buffer_write_int(int64_be, int64_t, sizeof(int64_t), htobe64, INT64_MIN, INT64_MAX);
}

#define LuaIO_buffer_write_float(name, type, bytes, endian, temp_type) do { \
  LuaIO_buffer_check_write_buffer(L, write_##name(n)); \
  LuaIO_buffer_check_memory(L, write_##name(n)); \
  \
  type n = (type)luaL_checknumber(L, 2); \
  \
  char *write_pos = buffer->write_pos; \
  char *read_pos = buffer->read_pos; \
  int rest_size = write_pos - read_pos; \
  assert(rest_size >= 0); \
  \
  char* start; \
  char* pos = write_pos + bytes; \
  if (pos <= buffer->end) { \
    temp_type *temp = (temp_type*)&n; \
    temp_type tmp = endian(*temp); \
    *((temp_type*)write_pos) = tmp; \
    buffer->write_pos += bytes; \
    lua_pushinteger(L, bytes); \
  } else if (pos <= (read_pos + buffer->capacity)) { \
    start = buffer->start; \
    LuaIO_memmove(start, read_pos, rest_size); \
    temp_type *temp = (temp_type*)&n; \
    temp_type tmp = endian(*temp); \
    write_pos = start + rest_size; \
    *((temp_type*)write_pos) = tmp; \
    buffer->read_pos = start; \
    buffer->write_pos = write_pos + bytes; \
    lua_pushinteger(L, bytes); \
  } else { \
    lua_pushinteger(L, LUAIO_EXCEED_BUFFER_CAPACITY); \
  } \
  \
  return 1; \
} while (0)

static int LuaIO_buffer_write_float_le(lua_State *L) {
  LuaIO_buffer_write_float(float_le, float, sizeof(float), htole32, uint32_t);
}

static int LuaIO_buffer_write_float_be(lua_State *L) {
  LuaIO_buffer_write_float(float_be, float, sizeof(float), htobe32, uint32_t);
}

static int LuaIO_buffer_write_double_le(lua_State *L) {
  LuaIO_buffer_write_float(double_le, double, sizeof(double), htole64, uint64_t);
}

static int LuaIO_buffer_write_double_be(lua_State *L) {
  LuaIO_buffer_write_float(double_be, double, sizeof(double), htobe64, uint64_t);
}

int luaopen_write_buffer(lua_State *L) {
  /*write buffer metatable*/
  lua_pushlightuserdata(L, &LuaIO_write_buffer_metatable_key);

  lua_createtable(L, 0, 23);
  LuaIO_function(LuaIO_buffer_capacity, "capacity")
  LuaIO_function(LuaIO_buffer_discard, "discard")
  LuaIO_function(LuaIO_buffer_write, "write")
  LuaIO_function(LuaIO_buffer_write_uint8, "write_uint8")
  LuaIO_function(LuaIO_buffer_write_int8, "write_int8")
  LuaIO_function(LuaIO_buffer_write_uint16_le, "write_uint16_le")
  LuaIO_function(LuaIO_buffer_write_uint16_be, "write_uint16_be")
  LuaIO_function(LuaIO_buffer_write_uint32_le, "write_uint32_le")
  LuaIO_function(LuaIO_buffer_write_uint32_be, "write_uint32_be")
  LuaIO_function(LuaIO_buffer_write_uint64_le, "write_uint64_le")
  LuaIO_function(LuaIO_buffer_write_uint64_be, "write_uint64_be")
  LuaIO_function(LuaIO_buffer_write_int16_le, "write_int16_le")
  LuaIO_function(LuaIO_buffer_write_int16_be, "write_int16_be")
  LuaIO_function(LuaIO_buffer_write_int32_le, "write_int32_le")
  LuaIO_function(LuaIO_buffer_write_int32_be, "write_int32_be")
  LuaIO_function(LuaIO_buffer_write_int64_le, "write_int64_le")
  LuaIO_function(LuaIO_buffer_write_int64_be, "write_int64_be")
  LuaIO_function(LuaIO_buffer_write_float_le, "write_float_le")
  LuaIO_function(LuaIO_buffer_write_float_be, "write_float_be")
  LuaIO_function(LuaIO_buffer_write_double_le, "write_double_le")
  LuaIO_function(LuaIO_buffer_write_double_be, "write_double_be")
  LuaIO_function(LuaIO_buffer_gc, "__gc")

  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  /*save write buffer metatable*/
  lua_rawset(L, LUA_REGISTRYINDEX);

  luaL_Reg lib[] = {
    { "new", LuaIO_write_buffer_new },
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
