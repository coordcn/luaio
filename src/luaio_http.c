/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "luaio.h"
#include "luaio_init.h"
#include "luaio_http_parser.h"

#define luaio_http_check_http_parser(L, name) \
  http_parser_t *parser = lua_touserdata(L, 1); \
  if (parser == NULL) { \
    return luaL_argerror(L, 1, "http_parser:"#name" error: http_parser must be userdata\n"); \
  }

#define luaio_http_check_buffer(L, name) \
  luaio_buffer_t *buffer = lua_touserdata(L, 2); \
  if (buffer == NULL || buffer->type != LUAIO_TYPE_READ_BUFFER) { \
    return luaL_argerror(L, 2, "http_parser:"#name" error: buffer must be ReadBuffer\n"); \
  }

static char luaio_http_parser_metatable_key;

/* @example: local parser = http.new_parser() */
static int luaio_http_new_parser(lua_State *L) {
  http_parser_t *parser = lua_newuserdata(L, sizeof(http_parser_t));
  if (parser == NULL) {
    lua_pushnil(L);
    return 1;
  }

  http_parser_init(parser);

  lua_pushlightuserdata(L, &luaio_http_parser_metatable_key);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_setmetatable(L, -2);
  return 1;
}

/* @example: local status, http_major, http_minor, error = http_parser:parse_status_line(read_buffer)
 * @param: read_buffer {userdate(ReadBuffer)}
 * @return: status {integer}
 * @return: http_major {integer}
 * @return: http_minor {integer}
 * @return: error {integer} 
 */
static int luaio_http_parser_parse_status_line(lua_State *L) {
  luaio_http_check_http_parser(L, parse_status_line(buffer));
  luaio_http_check_buffer(L, parse_status_line(buffer));

  int rest_size, dist;
  char *start, *last_pos;
  char *read_pos = buffer->read_pos;
  char *write_pos = buffer->write_pos;
  int ret = http_parse_status_line(parser, read_pos, write_pos);
  last_pos = parser->last_pos;

  if (ret == HTTP_OK) {
    lua_pushinteger(L, parser->status_code);
    lua_pushinteger(L, parser->http_major);
    lua_pushinteger(L, parser->http_minor);
    lua_pushinteger(L, ret);

    if (last_pos == write_pos) {
      start = buffer->start;
      buffer->read_pos = start;
      buffer->write_pos = start;
    } else {
      buffer->read_pos = last_pos;
    }
    parser->last_pos = NULL;

    return 4;
  } 
  
  if (ret == HTTP_AGAIN && write_pos == buffer->end) {
    start = buffer->start;

    /* start == read_pos   write_pos == end
     * |                           |
     * +++++++++++++++++++++++++++++
     */
    if (read_pos == start) {
      lua_pushnil(L);
      lua_pushnil(L);
      lua_pushnil(L);
      lua_pushinteger(L, HTTP_ERROR);
      return 4;
    }

    /* start  read_pos   write_pos == end
     * |        |                   |
     * ---------+++++++++++++++++++++
     */
    rest_size = write_pos - read_pos;
    dist = read_pos - start;

    luaio_memmove(start, read_pos, rest_size);
    buffer->read_pos = start;
    buffer->write_pos = start + rest_size;
    parser->last_pos = last_pos - dist;
  }
      
  lua_pushnil(L);
  lua_pushnil(L);
  lua_pushnil(L);
  lua_pushinteger(L, ret);
  return 4;
}

/* @example: local method, http_major, http_minor, url, error = http_parser:parse_request_line(read_buffer)
 * @param: read_buffer {userdate(ReadBuffer)}
 * @return: method {integer}
 * @return: http_major {integer}
 * @return: http_minor {integer}
 * @return: url {table} @http_parser:parse_request_line
 *    local url = {
 *      schema = {string}
 *      auth = {string}
 *      host = {string}
 *      port = {string}
 *      path = {string}
 *      query = {string}
 *      hash = {string}
 *    }
 * @return: error {integer} 
 */
static int luaio_http_parser_parse_request_line(lua_State *L) {
  luaio_http_check_http_parser(L, parse_request_line(buffer));
  luaio_http_check_buffer(L, parse_request_line(buffer));

  int rest_size, dist;
  char *start, *last_pos;
  char *read_pos = buffer->read_pos;
  char *write_pos = buffer->write_pos;
  int ret = http_parse_request_line(parser, read_pos, write_pos);
  last_pos = parser->last_pos;

  if (ret == HTTP_OK) {
    http_url_t *url = &parser->url;
    char *server = url->server.base;
    size_t length = url->server.len;
    if (server && length > 0) {
      ret = http_parse_host(url, server, length, parser->found_at);

      if (ret) goto BAD_REQUEST;
    }

    /* host must be present if there is a schema */
    /* parsing http:///toto will fail */
    if (url->schema.base != NULL && url->host.base == NULL) goto BAD_REQUEST;
  
    /* CONNECT requests can only contain "hostname:port" */
    if (parser->method == HTTP_CONNECT) {
      if (url->host.base != NULL && url->port.base != NULL) {
        if (url->userinfo.base != NULL
            || url->path.base != NULL 
            || url->query.base != NULL 
            || url->fragment.base != NULL) {
          goto BAD_REQUEST;
        }
      } else {
        goto BAD_REQUEST;
      }
    }

    lua_pushinteger(L, parser->method);
    lua_pushinteger(L, parser->http_major);
    lua_pushinteger(L, parser->http_minor);
    lua_createtable(L, 0, 7);
    if (url->schema.base != NULL) {
      luaio_setlstring("schema", url->schema.base, url->schema.len)
    }

    if (url->userinfo.base != NULL) {
      luaio_setlstring("auth", url->userinfo.base, url->userinfo.len)
    }

    if (url->host.base != NULL) {
      luaio_setlstring("host", url->host.base, url->host.len)
    }

    if (url->port.base != NULL) {
      luaio_setlstring("port", url->port.base, url->port.len)
    }

    if (url->path.base != NULL) {
      luaio_setlstring("path", url->path.base, url->path.len)
    }

    if (url->query.base != NULL) {
      luaio_setlstring("query", url->query.base, url->query.len)
    }

    if (url->fragment.base != NULL) {
      luaio_setlstring("hash", url->fragment.base, url->fragment.len)
    }
    lua_pushinteger(L, ret);

    if (last_pos == write_pos) {
      start = buffer->start;
      buffer->read_pos = start;
      buffer->write_pos = start;
    } else {
      buffer->read_pos = last_pos;
    }
    parser->last_pos = NULL;

    return 5;
  } 
  
  if (ret == HTTP_AGAIN && write_pos == buffer->end) {
    start = buffer->start;
 
    /* start == read_pos   write_pos == end
     * |                           |
     * +++++++++++++++++++++++++++++
     */
    if (read_pos == start) {
      lua_pushnil(L);
      lua_pushnil(L);
      lua_pushnil(L);
      lua_pushnil(L);
      lua_pushinteger(L, HTTP_REQUEST_URI_TOO_LARGE);
      return 5;
    }
 
    /* start  read_pos   write_pos == end
     * |        |                   |
     * ---------+++++++++++++++++++++
     */
    rest_size = write_pos - read_pos;
    dist = read_pos - start;
 
    luaio_memmove(start, read_pos, rest_size);
    buffer->read_pos = start;
    buffer->write_pos = start + rest_size;
    parser->last_pos = last_pos - dist;
    http_url_t *url = &parser->url;
    if (url->schema.base != NULL) {
      url->schema.base -= dist;
    }
 
    if (url->server.base != NULL) {
      url->server.base -= dist;
    }
 
    if (url->path.base != NULL) {
      url->path.base -= dist;
    }
 
    if (url->query.base != NULL) {
      url->query.base -= dist;
    }
 
    if (url->fragment.base != NULL) {
      url->fragment.base -= dist;
    }
  }
      
  lua_pushnil(L);
  lua_pushnil(L);
  lua_pushnil(L);
  lua_pushnil(L);
  lua_pushinteger(L, ret);
  return 5;

BAD_REQUEST:
  lua_pushnil(L);
  lua_pushnil(L);
  lua_pushnil(L);
  lua_pushnil(L);
  lua_pushinteger(L, HTTP_BAD_REQUEST);
  return 5;
}

/* @example: local error = http_parser:parse_headers(read_buffer, headers, cookies)
 * @param: read_buffer {userdate(ReadBuffer)}
 * @param: headers {table(map)} 
 * @param: cookies {table(array)}
 * @return: error {integer}
 * @example:
 *    local headers = {}
 *    local cookies = {}
 *    local error = http_parser:parse_headers(read_buffer, headers, cookies)
 */
static int luaio_http_parser_parse_headers(lua_State *L) {
  luaio_http_check_http_parser(L, parse_headers(buffer));
  luaio_http_check_buffer(L, parse_headers(buffer));

  http_buf_t headers[HTTP_MAX_HEADERS_PER_READ * 2];
  size_t nheader = 0;

  char *start, *last_pos;
  char *read_pos = buffer->read_pos;
  char *write_pos = buffer->write_pos;
  int ret = http_parse_headers(parser, read_pos, write_pos, headers, &nheader);
  last_pos = parser->last_pos;

  assert(nheader <= HTTP_MAX_HEADERS_PER_READ);
  if (nheader > 0) {
    size_t ncookie = lua_rawlen(L, 4);
  
    char *field, *value;
    size_t field_len, value_len, field_index, value_index;
    for (size_t i = 0; i < nheader; i++) {
      field_index = i << 1;
      value_index = field_index + 1;
      field = headers[field_index].base;
      field_len = headers[field_index].len;
      value = headers[value_index].base;
      value_len = headers[value_index].len;
  
      if (value != NULL) {
        if ((field_len == 6 && luaio_streq(field, "cookie", 6))
            || (field_len == 10 && luaio_streq(field, "set-cookie", 10))) {
          lua_pushlstring(L, value, value_len);
          lua_rawseti(L, 4, ncookie++);
        } else {
          lua_pushlstring(L, field, field_len);
          lua_pushlstring(L, value, value_len);
          lua_rawset(L, 3);
        }
      }
    }
  }

  if (ret == HTTP_OK || ret == HTTP_DONE) {
    if (last_pos == write_pos) {
      start = buffer->start;
      buffer->read_pos = start;
      buffer->write_pos = start;
    } else {
      buffer->read_pos = last_pos;
    }
    parser->last_pos = NULL;
  } else if (ret == HTTP_AGAIN) {
    if (write_pos == buffer->end) {
      start = buffer->start;
  
      /* start == read_pos   write_pos == end
       * |                           |
       * +++++++++++++++++++++++++++++
       */
      if (read_pos == start) {
        lua_pushinteger(L, HTTP_BAD_REQUEST);
        return 1;
      }
  
      /* start  read_pos   write_pos == end
       * |        |                   |
       * ---------+++++++++++++++++++++
       */
      uint8_t index = parser->index;
      if (index == 0) { 
        buffer->read_pos = start;
        buffer->write_pos = start;
        parser->last_pos = NULL;
      } else {
        read_pos = parser->field.base;
        int rest_size = write_pos - read_pos;
        luaio_memmove(start, read_pos, rest_size);
        buffer->read_pos = start;
        write_pos = start + rest_size;
        buffer->write_pos = write_pos;
        parser->last_pos = write_pos;
        
        if (index == 3) {
          char* value_base = parser->value.base;
          rest_size = value_base - read_pos;
          parser->value.base = start + rest_size;
        }

        parser->field.base = start;
      }
    }
  }
  
  lua_pushinteger(L, ret);
  return 1;
}

static int luaio_http_parser_reset(lua_State *L) {
  luaio_http_check_http_parser(L, reset());
  http_parser_init(parser);
  return 0;
}

static int luaio_http_parse_url(lua_State *L) {
  http_url_t url;
  size_t len;
  char *str = (char*)luaL_checklstring(L, 1, &len);
  int is_connect = lua_toboolean(L, 2);

  int ret = http_parse_url(&url, str, len, is_connect);
  if (ret) {
    lua_pushnil(L);
  } else {
    lua_createtable(L, 0, 7);
    if (url.schema.base != NULL) {
      luaio_setlstring("schema", url.schema.base, url.schema.len)
    }

    if (url.userinfo.base != NULL) {
      luaio_setlstring("auth", url.userinfo.base, url.userinfo.len)
    }

    if (url.host.base != NULL) {
      luaio_setlstring("host", url.host.base, url.host.len)
    }

    if (url.port.base != NULL) {
      luaio_setlstring("port", url.port.base, url.port.len)
    }

    if (url.path.base != NULL) {
      luaio_setlstring("path", url.path.base, url.path.len)
    }

    if (url.query.base != NULL) {
      luaio_setlstring("query", url.query.base, url.query.len)
    }

    if (url.fragment.base != NULL) {
      luaio_setlstring("hash", url.fragment.base, url.fragment.len)
    }
  }

  return 1;
}

static void luaio_http_setup_constants(lua_State *L) {
#define XX(num, name, _) \
  lua_pushinteger(L, num); \
  lua_setfield(L, -2, #name);

  HTTP_METHOD_MAP(XX)
#undef XX

  lua_pushinteger(L, HTTP_OK);
  lua_setfield(L, -2, "OK");

  lua_pushinteger(L, HTTP_DONE);
  lua_setfield(L, -2, "DONE");

  lua_pushinteger(L, HTTP_AGAIN);
  lua_setfield(L, -2, "AGAIN");

  lua_pushinteger(L, HTTP_ERROR);
  lua_setfield(L, -2, "ERROR");
}

int luaopen_http(lua_State *L) {
  /*http_Parser metatable*/
  luaL_Reg http_parser_mtlib[] = {
    { "parse_status_line", luaio_http_parser_parse_status_line },
    { "parse_request_line", luaio_http_parser_parse_request_line },
    { "parse_headers", luaio_http_parser_parse_headers },
    { "reset", luaio_http_parser_reset },
    { NULL, NULL }
  };

  lua_pushlightuserdata(L, &luaio_http_parser_metatable_key);
  luaL_newlib(L, http_parser_mtlib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_rawset(L, LUA_REGISTRYINDEX);

  luaL_Reg lib[] = {
    { "new_parser", luaio_http_new_parser },
    { "parse_url", luaio_http_parse_url },
    { "__newindex", luaio_cannot_change },
    { NULL, NULL }
  };

  lua_createtable(L, 0, 0);

  luaL_newlib(L, lib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushliteral(L, "metatable is protected.");
  lua_setfield(L, -2, "__metatable");
  luaio_http_setup_constants(L);

  lua_setmetatable(L, -2);

  return 1;
}
