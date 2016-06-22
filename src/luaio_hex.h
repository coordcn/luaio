/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_HEX_H
#define LUAIO_HEX_H

#include "luaio.h"

size_t luaio_hex_encode(char *dst, size_t dlen, const char *src, size_t slen);
size_t luaio_hex_decode(char *dst, size_t dlen, const char *src, size_t slen);

int luaopen_hex(lua_State *L);

#endif /* LUAIO_HEX_H */
