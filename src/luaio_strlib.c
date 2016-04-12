/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio.h"
#include "luaio_init.h"

#define LUAIO_STRLIB_SHORT_STRING_LENGTH    256
#define LUAIO_STRLIB_SHORT_PATTERN_LENGTH   8

/* U-00000000 - U-0000007F: 0xxxxxxx 
 * U-00000080 - U-000007FF: 110xxxxx 10xxxxxx 
 * U-00000800 - U-0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx 
 * U-00010000 - U-001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 
 * U-00200000 - U-03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 
 * U-04000000 - U-7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 
 */
static inline int utf8_bytes(unsigned char c) {
  switch (c >> 4) {
    case 0x0: case 0x1: case 0x2: case 0x3:
    case 0x4: case 0x5: case 0x6: case 0x7:
      return 1;
    case 0x8: case 0x9: case 0xa: case 0xb:
      return LUAIO_EBAD_UTF8_CHAR;
    case 0xc: case 0xd:
      return 2;
    case 0xe:
      return 3;
    case 0xf:
      switch (c & 0xf) {
        case 0x0: case 0x1: case 0x2: case 0x3:
        case 0x4: case 0x5: case 0x6: case 0x7:
          return 4;
        case 0x8: case 0x9: case 0xa: case 0xb:
          return 5;
        case 0xc: case 0xd:
          return 6;
        case 0xe: case 0xf: default:
          return LUAIO_EBAD_UTF8_CHAR;
      }
    default:
      return LUAIO_EBAD_UTF8_CHAR;
  }
}

static int luaio_strlib_utf8len(lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);

  int bytes = 0;
  int count = 0;
  const char *p = s;

  while (len) {
    bytes = utf8_bytes((unsigned char)*p);
    if (bytes > (int)len) bytes = LUAIO_EINCOMPLETE_UTF8_CHAR;
    if (bytes < 0) {
      lua_pushinteger(L, bytes);
      return 1;
    }
    
    count++;
    len -= bytes;
    p += bytes;
  }

  lua_pushinteger(L, count);
  return 1;
}

/* local parts, count = strlib.split("lua.io，中国。", "") 
 * parts = {"l","u","a",".","i","o","，","中","国","。"}
 * count = 10
 */
static int split_text_to_utf8_array(lua_State *L, 
                                    const char *text, 
                                    size_t textlen) {
  int bytes = 0;
  int count = 0;
  const char *p = text;

  lua_createtable(L, 0, 0);
  while (textlen) {
    bytes = utf8_bytes((unsigned char)*p);
    if (bytes > (int)textlen) bytes = LUAIO_EINCOMPLETE_UTF8_CHAR;
    if (bytes < 0) {
      lua_pop(L, 1);
      lua_pushnil(L);
      return bytes;
    }
    
    count++;
    lua_pushlstring(L, p, bytes); 
    lua_rawseti(L, -2, count);

    textlen -= bytes;
    p += bytes;
  }

  return count;
}

static int split_text_by_one_byte_char(lua_State* L, 
                                       const char *text,
                                       char c,
                                       size_t textlen) {
  int count = 0;
  const char *find;
  const char *start = text;
  const char *end = text + textlen;
  
  lua_createtable(L, 0, 0);
  find = luaio_memchr(start, c, textlen);
  if (find != NULL) {
    if (find != start) {
      count++;
      lua_pushlstring(L, start, find - start); 
      lua_rawseti(L, -2, count);
    }
    find++;
  } else {
    count++;
    lua_pushvalue(L, 1); 
    lua_rawseti(L, -2, count);
    return count;
  }

  while (find < end) {
    if (*find != c) {
      start = find;
      find++;
      if (find == end) {
        find = NULL;
      } else {
        find = luaio_memchr(find, c, end - find);
      }

      if (find != NULL) {
        count++;
        lua_pushlstring(L, start, find - start); 
        lua_rawseti(L, -2, count);
      } else {
        count++;
        lua_pushlstring(L, start, end - start); 
        lua_rawseti(L, -2, count);
        return count;
      }
    } else {
      find++;
    }
  }

  return count;
}

static int split_text_sunday(lua_State *L, 
                             const char *text, 
                             const char *pattern, 
                             size_t textlen, 
                             size_t patternlen) {
  uint32_t map[256];
  uint32_t step = patternlen + 1;

  size_t i;
  for (i = 0; i < 256; i++) {
    map[i] = step;
  }

  unsigned char c;
  for (i = 0; i < patternlen; i++) {
    c = pattern[i];
    map[c] = patternlen - i;
  }

  unsigned char next;
  int count = 0;
  const char *p = text;
  const char *end = text + textlen;
  const char *endcmp = end - patternlen;

  const char *start = NULL;
  lua_createtable(L, 0, 0);
  while (p <= endcmp) {
    /*if (luaio_memcmp(p, pattern, patternlen) == 0) {*/
    if (luaio_streq(p, pattern, patternlen)) {
      if (start != NULL) {
        count++;
        lua_pushlstring(L, start, p - start); 
        lua_rawseti(L, -2, count);
        start = NULL;
      }
      p += patternlen;
    } else {
      if (start == NULL) start = p;
      next = *(p + patternlen);
      p += map[next]; 
    }
  }

  if (start != NULL) {
    count++;
    lua_pushlstring(L, start, end - start); 
    lua_rawseti(L, -2, count);
  }

  return count;
}

static int split_text(lua_State *L, 
                      const char *text, 
                      const char *pattern, 
                      size_t textlen, 
                      size_t patternlen) {
  int count = 0;
  const char *find;
  const char *start = text;
  const char *end = text + textlen;
  const char *endcmp = end - patternlen;
  char c = *pattern;

  lua_createtable(L, 0, 0);
  find = luaio_memchr(start, c, textlen);
  if (find != NULL) {
    if (luaio_streq(find, pattern, patternlen)) {
      if (find != start) {
        count++;
        lua_pushlstring(L, start, find - start); 
        lua_rawseti(L, -2, count);
      }
      find += patternlen;
      start = find;
    } else {
      find++;
    }
  } else {
    count++;
    lua_pushvalue(L, 1); 
    lua_rawseti(L, -2, count);
    return count;
  }

  while (find <= endcmp) {
    find = luaio_memchr(find, c, end - find);
    if (find != NULL) {
      if (luaio_streq(find, pattern, patternlen)) {
        if (find != start) {
          count++;
          lua_pushlstring(L, start, find - start); 
          lua_rawseti(L, -2, count);
        }
        find += patternlen;
        start = find;
      } else {
        find++;
      }
    } else {
      count++;
      lua_pushlstring(L, start, end - start); 
      lua_rawseti(L, -2, count);
      return count;
    }
  }

  return count;
}

static int luaio_strlib_split(lua_State *L) {
  size_t textlen, patternlen;
  const char *text = luaL_checklstring(L, 1, &textlen);
  const char *pattern = luaL_checklstring(L, 2, &patternlen);

  if (textlen == 0 || textlen < patternlen) {
    lua_createtable(L, 0, 0);
    lua_pushvalue(L, 1);
    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, 1);
    return 2;
  }
 
  int count;
  if (patternlen == 0) {
    count = split_text_to_utf8_array(L, text, textlen); 
  } else if (patternlen == 1) {
    count = split_text_by_one_byte_char(L, text, *pattern, textlen); 
  } else {
    if (textlen < LUAIO_STRLIB_SHORT_STRING_LENGTH
        || patternlen < LUAIO_STRLIB_SHORT_PATTERN_LENGTH) {
      count = split_text(L, text, pattern, textlen, patternlen); 
    } else {
      count = split_text_sunday(L, text, pattern, textlen, patternlen);
    }
  }

  lua_pushinteger(L, count);
  return 2;
}

int luaopen_strlib(lua_State *L) {
  luaL_Reg lib[] = {
    { "utf8len", luaio_strlib_utf8len },
    { "split", luaio_strlib_split },
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
