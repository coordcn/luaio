/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#ifndef LUAIO_BASE64_H
#define LUAIO_BASE64_H

#include "luaio.h"

size_t luaio_base64_encode(char *dst, size_t dlen, const char *src, size_t slen);
size_t luaio_base64_decode(char *dst, size_t dlen,  const char *src, size_t slen);
size_t luaio_base64_encode_url(char *dst, size_t dlen, const char *src, size_t slen);
size_t luaio_base64_decode_url(char *dst, size_t dlen, const char *src, size_t slen);

int luaopen_base64(lua_State *L);

#endif /* LUAIO_BASE64_H */
