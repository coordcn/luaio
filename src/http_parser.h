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

/* Maximium header size allowed. If the macro is not defined
 * before including this header then the default is used. To
 * change the maximum header size, define the macro in the build
 * environment (e.g. -DHTTP_MAX_HEADER_SIZE=<value>). To remove
 * the effective limit on the size of the header, define the macro
 * to a very large number (e.g. -DHTTP_MAX_HEADER_SIZE=0x7fffffff)
 */
#ifndef HTTP_MAX_HEADER_SIZE
#define HTTP_MAX_HEADER_SIZE (32*1024)
#endif

#ifndef HTTP_MAX_HEADER_LINE_SIZE
#define HTTP_MAX_HEADER_LINE_SIZE (8*1024)
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

#define HTTP_OK         0
#define HTTP_AGAIN      -1
#define HTTP_ERROR      -2

typedef struct http_parser http_parser;
typedef struct http_url http_url;

typedef struct {
  char* start;
  char* end;
} http_string;

enum http_url_fields { 
  UF_SCHEMA           = 0,
  UF_USER             = 1,
  UF_PASS             = 2,
  UF_HOST             = 3,
  UF_PORT             = 4,
  UF_PATH             = 5,
  UF_QUERY            = 6,
  UF_FRAGMENT         = 7,
  UF_MAX              = 8
};

#define HTTP_MAX_HEADERS_PER_READ      32

struct http_parser {
  http_string url_parsed[UF_MAX];
  /*headers[0]: name*/
  /*headers[1]: value*/
  http_string headers[HTTP_MAX_HEADERS_PER_READ * 2];
  char* pos;
  uint32_t nread;
  uint32_t nread_line;
  uint32_t max_header_size;
  uint32_t max_header_line_size;
  uint16_t http_major;
  uint16_t http_minor;
  uint16_t status_code;
  uint16_t url_fields;
  uint8_t method;
  uint8_t http_errno;
  uint8_t state;
  uint8_t nheaders;
};

#define http_parser_init(parser) do {                                     \
  parser->pos = NULL;                                                     \
  parser->nread = 0;                                                      \
  parser->nread_line = 0;                                                 \
  parser->nheaders = 0;                                                   \
  parser->http_major = 0;                                                 \
  parser->http_minor = 0;                                                 \
  parser->status_code = 0;                                                \
  parser->url_fields = 0;                                                 \
  parser->method = 0;                                                     \
  parser->http_errno = 0;                                                 \
  parser->state = 0;                                                      \
} while (0)

#define http_parser_reset_after_read_response_line(parser) do {           \
  parser->pos = NULL;                                                     \
  parser->nread_line = 0;                                                 \
  parser->http_major = 0;                                                 \
  parser->http_minor = 0;                                                 \
  parser->status_code = 0;                                                \
  parser->state = 0;                                                      \
} while (0)

#define http_parser_reset_after_read_request_line(parser) do {            \
  parser->pos = NULL;                                                     \
  parser->nread_line = 0;                                                 \
  parser->http_major = 0;                                                 \
  parser->http_minor = 0;                                                 \
  parser->url_fields = 0;                                                 \
  parser->method = 0;                                                     \
  parser->state = 0;                                                      \
} while (0)

#define http_parser_reset_after_read_header_part(parser) do {             \
  parser->nheaders = 0;                                                   \
  parser->state = 0;                                                      \
} while (0)

#define http_parser_reset_after_read_header_all(parser) do {              \
  parser->pos = NULL;                                                     \
  parser->nread = 0;                                                      \
  parser->nread_line = 0;                                                 \
  parser->nheaders = 0;                                                   \
  parser->state = 0;                                                      \
} while (0)

int http_parse_response_line(http_parser* parser, const char* data, const char* last);
int http_parse_request_line(http_parser* parser, const char* data, const char* last);
int http_parse_headers(http_parser* parser, const char* data, const char* last);

#ifdef __cplusplus
}
#endif
#endif /* LINKS_HTTP_PARSER_H */
