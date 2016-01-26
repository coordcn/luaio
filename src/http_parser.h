/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: http parser base on http_parser and nginx
 * @reference: http://github.com/joyent/http_parser
 *             src/http/ngx_http_parser.c
 */

#ifndef LINKS_HTTP_PARSER_H
#define LINKS_HTTP_PARSER_H
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

#ifndef HTTP_MAX_HEADER_LINE_SIZE
#define HTTP_MAX_HEADER_LINE_SIZE (16*1024)
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

enum http_method
  {
#define XX(num, name, string) HTTP_##name = num,
  HTTP_METHOD_MAP(XX)
#undef XX
  };

/* Map for errno-related constants
 * 
 * The provided argument should be a macro that takes 2 arguments.
 */
#define HTTP_ERRNO_MAP(XX)                                           \
  /* No error */                                                     \
  XX(OK, "success")                                                  \
                                                                     \
  /* Parsing-related errors */                                       \
  XX(INVALID_EOF_STATE, "stream ended at an unexpected time")        \
  XX(HEADER_OVERFLOW,                                                \
     "too many header bytes seen; overflow detected")                \
  XX(HEADER_LINE_OVERFLOW,                                                \
     "too many header line bytes seen; overflow detected")                \
  XX(CLOSED_CONNECTION,                                              \
     "data received after completed connection: close message")      \
  XX(INVALID_VERSION, "invalid HTTP version")                        \
  XX(INVALID_STATUS, "invalid HTTP status code")                     \
  XX(INVALID_METHOD, "invalid HTTP method")                          \
  XX(INVALID_URL, "invalid URL")                                     \
  XX(INVALID_HOST, "invalid host")                                   \
  XX(INVALID_PORT, "invalid port")                                   \
  XX(INVALID_PATH, "invalid path")                                   \
  XX(INVALID_QUERY_STRING, "invalid query string")                   \
  XX(INVALID_FRAGMENT, "invalid fragment")                           \
  XX(LF_EXPECTED, "LF character expected")                           \
  XX(INVALID_HEADER_TOKEN, "invalid character in header")            \
  XX(INVALID_CONTENT_LENGTH,                                         \
     "invalid character in content-length header")                   \
  XX(INVALID_CHUNK_SIZE,                                             \
     "invalid character in chunk size header")                       \
  XX(INVALID_CONSTANT, "invalid constant string")                    \
  XX(INVALID_INTERNAL_STATE, "encountered unexpected internal state")\
  XX(STRICT, "strict mode assertion failed")                         \
  XX(PAUSED, "parser is paused")                                     \
  XX(UNKNOWN, "an unknown error occurred")


/* Define HPE_* values for each errno value above */
#define HTTP_ERRNO_GEN(n, s) HPE_##n,
enum http_errno {
  HTTP_ERRNO_MAP(HTTP_ERRNO_GEN)
};
#undef HTTP_ERRNO_GEN


/* Get an http_errno value from an http_parser */
#define HTTP_PARSER_ERRNO(p)            ((enum http_errno) (p)->http_errno)

/*parse completed*/
#define HTTP_OK         0
/*parsed HTTP_MAX_HEADERS_PER_READ headers*/
#define HTTP_DONE       -1
/*parse error*/
#define HTTP_ERROR      -2
/*need more data*/
#define HTTP_AGAIN      -3

typedef struct http_parser http_parser;
typedef struct http_url http_url;

typedef struct {
  char    *base;
  size_t  len;
} http_buf_t;

struct http_url {
  http_buf_t schema;
  /*USERINFO@HOST:PORT*/
  http_buf_t server;
  http_buf_t path;
  http_buf_t query;
  http_buf_t fragment;
  http_buf_t userinfo;
  http_buf_t host;
  http_buf_t port;
};

#define HTTP_MAX_HEADERS_PER_READ      32
struct http_parser {
  http_url    url;
  /*headers[0]: field*/
  /*headers[1]: value*/
  http_buf_t  headers[HTTP_MAX_HEADERS_PER_READ * 2];
  char*       last_pos;
  size_t      nread;
  size_t      max_header_line_size;
  uint16_t    http_major;
  uint16_t    http_minor;
  uint16_t    status_code;
  uint8_t     method;
  uint8_t     index;
  uint8_t     http_errno;
  uint8_t     state;
  /*current headers_num * 2*/
  uint8_t     nbuf;
  uint8_t     found_at;
};

void http_parser_init(http_parser *parser, size_t max_header_line_size);

int http_parse_status_line(http_parser *parser, char *data, char *last);
int http_parse_request_line(http_parser *parser, char *data, char *last);
int http_parse_headers(http_parser *parser, char *data, char *last);

const char *http_method_str(enum http_method m);
const char *http_errno_name(enum http_errno err);
const char *http_errno_description(enum http_errno err);

int http_parse_host(http_url *url, char *data, size_t len, uint8_t found_at);
int http_parse_url(http_url *url, char *data, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* LINKS_HTTP_PARSER_H */
