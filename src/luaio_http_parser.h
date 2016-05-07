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
  XX(0,  UNKNOWN,     UNKNOWN)      \
  XX(1,  GET,         GET)          \
  XX(2,  POST,        POST)         \
  XX(3,  HEAD,        HEAD)         \
  XX(4,  PUT,         PUT)          \
  XX(5,  DELETE,      DELETE)       \
  /* pathological */                \
  XX(6,  CONNECT,     CONNECT)      \
  XX(7,  OPTIONS,     OPTIONS)      \
  XX(8,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(9,  COPY,        COPY)         \
  XX(10,  LOCK,        LOCK)         \
  XX(11, MKCOL,       MKCOL)        \
  XX(12, MOVE,        MOVE)         \
  XX(13, PROPFIND,    PROPFIND)     \
  XX(14, PROPPATCH,   PROPPATCH)    \
  XX(15, SEARCH,      SEARCH)       \
  XX(16, UNLOCK,      UNLOCK)       \
  XX(17, BIND,        BIND)         \
  XX(18, REBIND,      REBIND)       \
  XX(19, UNBIND,      UNBIND)       \
  XX(20, ACL,         ACL)          \
  /* subversion */                  \
  XX(21, REPORT,      REPORT)       \
  XX(22, MKACTIVITY,  MKACTIVITY)   \
  XX(23, CHECKOUT,    CHECKOUT)     \
  XX(24, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(25, MSEARCH,     M-SEARCH)     \
  XX(26, NOTIFY,      NOTIFY)       \
  XX(27, SUBSCRIBE,   SUBSCRIBE)    \
  XX(28, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(29, PATCH,       PATCH)        \
  XX(30, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(31, MKCALENDAR,  MKCALENDAR)   \

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

typedef struct {
  char    *base;
  size_t  len;
} http_buf_t;

typedef struct {
  http_buf_t schema;
  http_buf_t userinfo;
  http_buf_t host;
  http_buf_t port;
  http_buf_t path;
  http_buf_t query;
  http_buf_t fragment;
  /*USERINFO@HOST:PORT*/
  http_buf_t server;
} http_url_t;

#define HTTP_MAX_STATUS_LINE_SIZE     (16 * 1024)
#define HTTP_MAX_REQUEST_LINE_SIZE    (16 * 1024)
#define HTTP_MAX_HEADER_SIZE          (64 * 1024)
#define HTTP_MAX_HEADERS              256
#define HTTP_MAX_HEADERS_PER_READ     64

typedef struct {
  http_url_t  url;
  http_buf_t  field;
  http_buf_t  value;
  char        *last_pos;
  uint32_t    nread;
  uint16_t    http_major;
  uint16_t    http_minor;
  uint16_t    status_code;
  uint16_t    nheader;
  uint8_t     method;
  uint8_t     state;
  uint8_t     index;
  uint8_t     found_at;
} http_parser_t;

void http_parser_init(http_parser_t *parser);

int http_parse_status_line(http_parser_t *parser, char *data, char *last);

int http_parse_request_line(http_parser_t *parser, char *data, char *last);

int http_parse_headers(http_parser_t *parser, 
                       char *data, 
                       char *last, 
                       http_buf_t* headers, 
                       size_t *nheader);

int http_parse_host(http_url_t *url, 
                    char *data, 
                    size_t len, 
                    uint8_t found_at);

int http_parse_url(http_url_t *url, char *data, size_t len, int is_connect);

#ifdef __cplusplus
}
#endif
#endif /* LUAIO_HTTP_PARSER_H */
