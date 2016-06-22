/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio_hex.h"
#include "luaio_stack_buffer.h"

size_t luaio_hex_encode_internal(char *dst, size_t dlen, const char *src, size_t slen) {
  assert(dlen >= slen * 2);
  
  static const char hex[] = "0123456789abcdef";

  uint32_t k = 0;
  for (uint32_t i = 0; i  < slen; i++) {
    uint8_t val = (uint8_t)(src[i]);
    dst[k + 0] = hex[val >> 4];
    dst[k + 1] = hex[val & 15];
    k += 2;
  }

  return slen * 2;
}

static unsigned hex2bin(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
  if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
  return (unsigned)(-1);
}

size_t luaio_hex_decode_internal(char *dst, size_t dlen, const char *src, size_t slen) {
  assert(dlen >= slen / 2);

  size_t k = 0;
  dlen = slen / 2;
  for (size_t i = 0; i < dlen; i++) {
    unsigned a = hex2bin(src[k + 0]);
    unsigned b = hex2bin(src[k + 1]);
    if (!~a || !~b) return i;

    dst[i] = a << 4 + b;
    k += 2;
  }

  return dlen;
}

static int luaio_hex_encode(lua_State *L) {
  size_t slen;
  const char *src = luaL_checklstring(L, 1, &slen);

  size_t dlen = slen * 2;
  luaio_stack_buffer_t stack_buf;
  char *buf = luaio_stack_buffer_init(&stack_buf, dlen);
  size_t dlen = luaio_hex_encode_internal(dst, dlen, src, slen);
  
  lua_pushlstring(L, dst, dlen);
  luaio_stack_buffer_free(&stack_buf);
  return 1;
}

static int luaio_hex_decode(lua_State *L) {
  size_t slen;
  const char *src = luaL_checklstring(L, 1, &slen);

  size_t dlen = slen / 2;
  luaio_stack_buffer_t stack_buf;
  char *buf = luaio_stack_buffer_init(&stack_buf, dlen);
  size_t dlen = luaio_hex_decode_internal(dst, dlen, src, slen);
  
  lua_pushlstring(L, dst, dlen);
  luaio_stack_buffer_free(&stack_buf);
  return 1;
}

int luaopen_hex(lua_State *L) {
  luaL_Reg lib[] = {
    { "encode", luaio_hex_encode },
    { "decode", luaio_hex_decode },
    { "__newindex", luaio_cannot_change },
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
