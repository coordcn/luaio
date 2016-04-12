/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: http parser base on http_parser and nginx
 * @reference: http://github.com/joyent/http_parser
 *             src/http/ngx_http_parser.c
 */

#ifndef LUAIO_HTTP_PARSER_H
#define LUAIO_HTTP_PARSER_H
#ifdef __cplusplus
extern "C" {
#endif

/* Also update SONAME in the Makefile whenever you change these. */
#define HTTP_PARSER_VERSION_MAJOR 0
#define HTTP_PARSER_VERSION_MINOR 1
#define HTTP_PARSER_VERSION_PATCH 0

#include <sys/types.h>
#if defined(_WIN32) && !defined(__MINGW32__) && \
  (!defined(_MSC_VER) || _MSC_VER<1600) && !defined(__WINE__)
#include <BaseTsd.h>
#include <stddef.h>
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

/* Compile with -DHTTP_PARSER_STRICT=0 to make less checks, but run
 * faster
 */
#ifndef HTTP_PARSER_STRICT
# define HTTP_PARSER_STRICT 1
#endif

/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \

enum http_method {
#define XX(num, name, string) HTTP_##name = num,
  HTTP_METHOD_MAP(XX)
#undef XX
};


/*parse completed*/
#define HTTP_OK                         0
/*parsed HTTP_MAX_HEADERS_PER_READ headers*/
#define HTTP_DONE                       -1
/*need more data*/
#define HTTP_AGAIN                      -2
/*parse error*/
#define HTTP_ERROR                      -3
/*parse Bad Request*/
#define HTTP_BAD_REQUEST                400
/*parse Request-URI Too Large*/
#define HTTP_REQUEST_URI_TOO_LARGE      414

typedef struct http_parser http_parser;
typedef struct http_url http_url;

typedef struct {
  char    *base;
  size_t  len;
} http_buf_t;

struct http_url {
  http_buf_t schema;
  http_buf_t userinfo;
  http_buf_t host;
  http_buf_t port;
  http_buf_t path;
  http_buf_t query;
  http_buf_t fragment;
  /*USERINFO@HOST:PORT*/
  http_buf_t server;
};

#define HTTP_MAX_HEADERS_PER_READ      16
struct http_parser {
  http_url    url;
  /*headers[0]: field*/
  /*headers[1]: value*/
  http_buf_t  headers[HTTP_MAX_HEADERS_PER_READ * 2];
  char        *last_pos;
  uint32_t    nread;
  uint32_t    max_header_line_size;
  uint16_t    http_major;
  uint16_t    http_minor;
  uint16_t    status_code;
  uint8_t     method;
  uint8_t     state;
  /*current headers_num * 2*/
  uint8_t     nbuf;
  uint8_t     index;
  uint8_t     found_at;
};

void http_parser_init(http_parser *parser);

int http_parse_status_line(http_parser *parser, char *data, char *last);
int http_parse_request_line(http_parser *parser, char *data, char *last);
int http_parse_headers(http_parser *parser, char *data, char *last);

int http_parse_host(http_url *url, char *data, size_t len, uint8_t found_at);
int http_parse_url(http_url *url, char *data, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* LUAIO_HTTP_PARSER_H */
