/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: http parser base on http_parser and nginx
 * @reference: http://github.com/joyent/http_parser
 *             src/http/ngx_http_parser.c
 */

#include "http_parser.h"
#include <assert.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef ULLONG_MAX
# define ULLONG_MAX ((uint64_t) -1) /* 2^64-1 */
#endif

#ifndef MIN
# define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef BIT_AT
# define BIT_AT(a, i)                                                \
  (!!((unsigned int) (a)[(unsigned int) (i) >> 3] &                  \
      (1 << ((unsigned int) (i) & 7))))
#endif

#define CURRENT_STATE() p_state
#define UPDATE_STATE(V) p_state = (enum state) (V);
#define RETURN(V)                                                      \
  do {                                                                 \
    parser->state = CURRENT_STATE();                                   \
    parser->last_pos = p + 1;                                          \
    return (V);                                                        \
  } while (0);


#ifdef __GNUC__
# define LIKELY(X) __builtin_expect(!!(X), 1)
# define UNLIKELY(X) __builtin_expect(!!(X), 0)
#else
# define LIKELY(X) (X)
# define UNLIKELY(X) (X)
#endif


#define PROXY_CONNECTION "proxy-connection"
#define CONNECTION "connection"
#define CONTENT_LENGTH "content-length"
#define TRANSFER_ENCODING "transfer-encoding"
#define UPGRADE "upgrade"
#define CHUNKED "chunked"
#define KEEP_ALIVE "keep-alive"
#define CLOSE "close"


static const char *method_strings[] =
{
#define XX(num, name, string) #string,
  HTTP_METHOD_MAP(XX)
#undef XX
};


/* Tokens as defined by rfc 2616. Also lowercases them.
 *        token       = 1*<any CHAR except CTLs or separators>
 *     separators     = "(" | ")" | "<" | ">" | "@"
 *                    | "," | ";" | ":" | "\" | <">
 *                    | "/" | "[" | "]" | "?" | "="
 *                    | "{" | "}" | SP | HT
 */
static const char tokens[256] = {
  /*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
  0,       0,       0,       0,       0,       0,       0,       0,
  /*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
  0,       0,       0,       0,       0,       0,       0,       0,
  /*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
  0,       0,       0,       0,       0,       0,       0,       0,
  /*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
  0,       0,       0,       0,       0,       0,       0,       0,
  /*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
  0,      '!',      0,      '#',     '$',     '%',     '&',    '\'',
  /*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
  0,       0,      '*',     '+',      0,      '-',     '.',      0,
  /*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
  '0',     '1',     '2',     '3',     '4',     '5',     '6',     '7',
  /*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
  '8',     '9',      0,       0,       0,       0,       0,       0,
  /*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
  0,      'a',     'b',     'c',     'd',     'e',     'f',     'g',
  /*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
  'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
  /*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
  'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
  /*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
  'x',     'y',     'z',      0,       0,       0,      '^',     '_',
  /*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
  '`',     'a',     'b',     'c',     'd',     'e',     'f',     'g',
  /* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
  'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
  /* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
  'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
  /* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
  'x',     'y',     'z',      0,      '|',      0,      '~',       0 };


static const int8_t unhex[256] =
{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    , 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1
    ,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1
    ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    ,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1
    ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};


#if HTTP_PARSER_STRICT
# define T(v) 0
#else
# define T(v) v
#endif


static const uint8_t normal_url_char[32] = {
  /*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
  0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
  /*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
  0    | T(2)   |   0    |   0    | T(16)  |   0    |   0    |   0,
  /*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
  0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
  /*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
  0    |   0    |   0    |   0    |   0    |   0    |   0    |   0,
  /*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
  0    |   2    |   4    |   0    |   16   |   32   |   64   |  128,
  /*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
  /*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
  /*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |   0,
  /*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
  /*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
  /*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
  /*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
  /*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
  /* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
  /* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |  128,
  /* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
  1    |   2    |   4    |   8    |   16   |   32   |   64   |   0, };

#undef T

enum state {
  s_start = 0,

  s_res_H,
  s_res_HT,
  s_res_HTT,
  s_res_HTTP,
  s_res_first_http_major,
  s_res_http_major,
  s_res_first_http_minor,
  s_res_http_minor,
  s_res_first_status_code,
  s_res_status_code,
  s_res_status_start,
  s_res_status,
  s_res_line_almost_done,

  s_req_method,
  s_req_spaces_before_url,
  s_req_schema,
  s_req_schema_slash,
  s_req_schema_slash_slash,
  s_req_server_start,
  s_req_server,
  s_req_path,
  s_req_query_string_start,
  s_req_query_string,
  s_req_fragment_start,
  s_req_fragment,
  s_req_http_start,
  s_req_http_H,
  s_req_http_HT,
  s_req_http_HTT,
  s_req_http_HTTP,
  s_req_first_http_major,
  s_req_http_major,
  s_req_first_http_minor,
  s_req_http_minor,
  s_req_line_almost_done,

  s_header_field,
  s_header_space_before_value,
  s_header_value,
  s_header_space_after_value,
  s_header_almost_done,
  s_headers_almost_done
};

enum http_host_state {
  s_http_userinfo_start,
  s_http_userinfo,
  s_http_host_start,
  s_http_host_v6_start,
  s_http_host,
  s_http_host_v6,
  s_http_host_v6_end,
  s_http_host_v6_zone_start,
  s_http_host_v6_zone,
  s_http_host_port_start,
  s_http_host_port
};

/* Macros for character classes; depends on strict-mode  */
#define CR                  '\r'
#define LF                  '\n'
#define LOWER(c)            (unsigned char)(c | 0x20)
#define IS_ALPHA(c)         (LOWER(c) >= 'a' && LOWER(c) <= 'z')
#define IS_NUM(c)           ((c) >= '0' && (c) <= '9')
#define IS_ALPHANUM(c)      (IS_ALPHA(c) || IS_NUM(c))
#define IS_HEX(c)           (IS_NUM(c) || (LOWER(c) >= 'a' && LOWER(c) <= 'f'))
#define IS_MARK(c)          ((c) == '-' || (c) == '_' || (c) == '.' || \
    (c) == '!' || (c) == '~' || (c) == '*' || (c) == '\'' || (c) == '(' || \
    (c) == ')')
#define IS_USERINFO_CHAR(c) (IS_ALPHANUM(c) || IS_MARK(c) || (c) == '%' || \
    (c) == ';' || (c) == ':' || (c) == '&' || (c) == '=' || (c) == '+' || \
    (c) == '$' || (c) == ',')

#define STRICT_TOKEN(c)     (tokens[(unsigned char)c])

#if HTTP_PARSER_STRICT
#define TOKEN(c)            (tokens[(unsigned char)c])
#define IS_URL_CHAR(c)      (BIT_AT(normal_url_char, (unsigned char)c))
#define IS_HOST_CHAR(c)     (IS_ALPHANUM(c) || (c) == '.' || (c) == '-')
#else
#define TOKEN(c)            ((c == ' ') ? ' ' : tokens[(unsigned char)c])
#define IS_URL_CHAR(c)                                                         \
  (BIT_AT(normal_url_char, (unsigned char)c) || ((c) & 0x80))
#define IS_HOST_CHAR(c)                                                        \
  (IS_ALPHANUM(c) || (c) == '.' || (c) == '-' || (c) == '_')
#endif


#if HTTP_PARSER_STRICT
# define STRICT_CHECK(cond, error)                                     \
  do {                                                                 \
    if (cond) {                                                        \
      RETURN(error);                                                   \
    }                                                                  \
  } while (0)
#else
# define STRICT_CHECK(cond, error)
#endif


void http_parser_init(http_parser *parser) {
  parser->url.schema.base = NULL;
  parser->url.schema.len = 0;
  parser->url.userinfo.base = NULL;
  parser->url.userinfo.len = 0;
  parser->url.host.base = NULL;
  parser->url.host.len = 0;
  parser->url.port.base = NULL;
  parser->url.port.len = 0;
  parser->url.path.base = NULL;
  parser->url.path.len = 0;
  parser->url.query.base = NULL;
  parser->url.query.len = 0;
  parser->url.fragment.base = NULL;
  parser->url.fragment.len = 0;
  parser->url.server.base = NULL;
  parser->url.server.len = 0;

  parser->last_pos = NULL;
  parser->nread = 0;
  parser->http_major = 0;
  parser->http_minor = 0;
  parser->status_code = 0;
  parser->method = 0;
  parser->state = 0;
  parser->nbuf = 0;
  parser->index = 0;
  parser->found_at = 0;
}

int http_parse_status_line(http_parser *parser, char *data, char *last) {
  char ch;
  char *p;
  enum state p_state = (enum state)parser->state;

  assert(data <= last);
  p = parser->last_pos ? parser->last_pos : data;
  assert(p >= data || p <= last);

  for (; p < last; p++) {
    ch = *p;
    parser->nread += 1;
    if (UNLIKELY(parser->nread > parser->max_header_line_size)) {
      RETURN(HTTP_ERROR);
    }

    switch (CURRENT_STATE()) {
      case s_start:
      {
        switch (ch) {
          case 'H':
            UPDATE_STATE(s_res_H);
            break;

          case CR:
          case LF:
            break;

          default:
            RETURN(HTTP_ERROR);
        }

        break;
      }

      case s_res_H:
        STRICT_CHECK(ch != 'T', HTTP_ERROR);
        UPDATE_STATE(s_res_HT); 
        break;

      case s_res_HT:
        STRICT_CHECK(ch != 'T', HTTP_ERROR);
        UPDATE_STATE(s_res_HTT);
        break;

      case s_res_HTT:
        STRICT_CHECK(ch != 'P', HTTP_ERROR);
        UPDATE_STATE(s_res_HTTP);
        break;

      case s_res_HTTP:
        STRICT_CHECK(ch != '/', HTTP_ERROR);
        UPDATE_STATE(s_res_first_http_major);
        break;

      case s_res_first_http_major:
        if (UNLIKELY(ch < '0' || ch > '9')) {
          RETURN(HTTP_ERROR);
        }

        parser->http_major = ch - '0';
        UPDATE_STATE(s_res_http_major);
        break;

        /* major HTTP version or dot */
      case s_res_http_major:
      {
        if (ch == '.') {
          UPDATE_STATE(s_res_first_http_minor);
          break;
        }

        if (!IS_NUM(ch)) {
          RETURN(HTTP_ERROR);
        }

        parser->http_major *= 10;
        parser->http_major += ch - '0';

        if (UNLIKELY(parser->http_major > 999)) {
          RETURN(HTTP_ERROR);
        }

        break;
      }

      /* first digit of minor HTTP version */
      case s_res_first_http_minor:
        if (UNLIKELY(!IS_NUM(ch))) {
          RETURN(HTTP_ERROR);
        }

        parser->http_minor = ch - '0';
        UPDATE_STATE(s_res_http_minor);
        break;

      /* minor HTTP version or end of request line */
      case s_res_http_minor:
      {
        if (ch == ' ') {
          UPDATE_STATE(s_res_first_status_code);
          break;
        }

        if (UNLIKELY(!IS_NUM(ch))) {
          RETURN(HTTP_ERROR);
        }

        parser->http_minor *= 10;
        parser->http_minor += ch - '0';

        if (UNLIKELY(parser->http_minor > 999)) {
          RETURN(HTTP_ERROR);
        }

        break;
      }

      case s_res_first_status_code:
      {
        if (!IS_NUM(ch)) {
          if (ch == ' ') break;
          RETURN(HTTP_ERROR);
        }

        parser->status_code = ch - '0';
        UPDATE_STATE(s_res_status_code);
        break;
      }

      case s_res_status_code:
      {
        if (!IS_NUM(ch)) {
          switch (ch) {
            case ' ':
              UPDATE_STATE(s_res_status_start);
              break;
            case CR:
              UPDATE_STATE(s_res_line_almost_done);
              break;
            case LF:
              parser->nread = 0;
              UPDATE_STATE(s_start);
              RETURN(HTTP_OK);
            default:
              RETURN(HTTP_ERROR);
          }
          break;
        }

        parser->status_code *= 10;
        parser->status_code += ch - '0';
        if (UNLIKELY(parser->status_code > 999)) {
          RETURN(HTTP_ERROR);
        }

        break;
      }

      case s_res_status_start:
      {
        if (ch == CR) {
          UPDATE_STATE(s_res_line_almost_done);
          break;
        }

        if (ch == LF) {
          parser->nread = 0;
          UPDATE_STATE(s_start);
          RETURN(HTTP_OK);
        }

        UPDATE_STATE(s_res_status);
        break;
      }

      case s_res_status:
        if (ch == CR) {
          UPDATE_STATE(s_res_line_almost_done);
          break;
        }

        if (ch == LF) {
          parser->nread = 0;
          UPDATE_STATE(s_start);
          RETURN(HTTP_OK);
        }

        break;

      case s_res_line_almost_done:
        STRICT_CHECK(ch != LF, HTTP_ERROR);
        parser->nread = 0;
        UPDATE_STATE(s_start);
        RETURN(HTTP_OK);

      default:
        assert(0 && "unhandled state");
        RETURN(HTTP_ERROR);
    }
  }

  parser->state = CURRENT_STATE();
  parser->last_pos = p;
  return HTTP_AGAIN;
}

int http_parse_request_line(http_parser *parser, char *data, char *last) {
  char ch;
  char *p;
  enum state p_state = (enum state)parser->state;

  assert(data <= last);
  p = parser->last_pos ? parser->last_pos : data;
  assert(p >= data || p <= last);

  for (; p < last; p++) {
    ch = *p;
    parser->nread += 1;
    if (UNLIKELY(parser->nread > parser->max_header_line_size)) {
      RETURN(HTTP_REQUEST_URI_TOO_LARGE);
    }

    switch (CURRENT_STATE()) {
      case s_start:
      {
        if (ch == CR || ch == LF)
          break;

        if (UNLIKELY(!IS_ALPHA(ch))) {
          RETURN(HTTP_BAD_REQUEST);
        }

        parser->method = (enum http_method) 0;
        parser->index = 1;
        switch (ch) {
          case 'C': parser->method = HTTP_CONNECT; /* or COPY, CHECKOUT */ break;
          case 'D': parser->method = HTTP_DELETE; break;
          case 'G': parser->method = HTTP_GET; break;
          case 'H': parser->method = HTTP_HEAD; break;
          case 'L': parser->method = HTTP_LOCK; break;
          case 'M': parser->method = HTTP_MKCOL; /* or MOVE, MKACTIVITY, MERGE, M-SEARCH, MKCALENDAR */ break;
          case 'N': parser->method = HTTP_NOTIFY; break;
          case 'O': parser->method = HTTP_OPTIONS; break;
          case 'P': parser->method = HTTP_POST;/* or PROPFIND|PROPPATCH|PUT|PATCH|PURGE */ break;
          case 'R': parser->method = HTTP_REPORT; break;
          case 'S': parser->method = HTTP_SUBSCRIBE; /* or SEARCH */ break;
          case 'T': parser->method = HTTP_TRACE; break;
          case 'U': parser->method = HTTP_UNLOCK; /* or UNSUBSCRIBE */ break;
          default:
                    RETURN(HTTP_BAD_REQUEST);
        }
        UPDATE_STATE(s_req_method);

        break;
      }

      case s_req_method:
      {
        const char *matcher;
        if (UNLIKELY(ch == '\0')) {
          RETURN(HTTP_BAD_REQUEST);
        }

        matcher = method_strings[parser->method];
        if (ch == ' ' && matcher[parser->index] == '\0') {
          parser->index = 0;
          UPDATE_STATE(s_req_spaces_before_url);
        } else if (ch == matcher[parser->index]) {
          ; /* nada */
        } else if (parser->method == HTTP_CONNECT) {
          if (parser->index == 1 && ch == 'H') {
            parser->method = HTTP_CHECKOUT;
          } else if (parser->index == 2  && ch == 'P') {
            parser->method = HTTP_COPY;
          } else {
            RETURN(HTTP_BAD_REQUEST);
          }
        } else if (parser->method == HTTP_MKCOL) {
          if (parser->index == 1 && ch == 'O') {
            parser->method = HTTP_MOVE;
          } else if (parser->index == 1 && ch == 'E') {
            parser->method = HTTP_MERGE;
          } else if (parser->index == 1 && ch == '-') {
            parser->method = HTTP_MSEARCH;
          } else if (parser->index == 2 && ch == 'A') {
            parser->method = HTTP_MKACTIVITY;
          } else if (parser->index == 3 && ch == 'A') {
            parser->method = HTTP_MKCALENDAR;
          } else {
            RETURN(HTTP_BAD_REQUEST);
          }
        } else if (parser->method == HTTP_SUBSCRIBE) {
          if (parser->index == 1 && ch == 'E') {
            parser->method = HTTP_SEARCH;
          } else {
            RETURN(HTTP_BAD_REQUEST);
          }
        } else if (parser->index == 1 && parser->method == HTTP_POST) {
          if (ch == 'R') {
            parser->method = HTTP_PROPFIND; /* or HTTP_PROPPATCH */
          } else if (ch == 'U') {
            parser->method = HTTP_PUT; /* or HTTP_PURGE */
          } else if (ch == 'A') {
            parser->method = HTTP_PATCH;
          } else {
            RETURN(HTTP_BAD_REQUEST);
          }
        } else if (parser->index == 2) {
          if (parser->method == HTTP_PUT) {
            if (ch == 'R') {
              parser->method = HTTP_PURGE;
            } else {
              RETURN(HTTP_BAD_REQUEST);
            }
          } else if (parser->method == HTTP_UNLOCK) {
            if (ch == 'S') {
              parser->method = HTTP_UNSUBSCRIBE;
            } else {
              RETURN(HTTP_BAD_REQUEST);
            }
          } else {
            RETURN(HTTP_BAD_REQUEST);
          }
        } else if (parser->index == 4 && parser->method == HTTP_PROPFIND && ch == 'P') {
          parser->method = HTTP_PROPPATCH;
        } else {
          RETURN(HTTP_BAD_REQUEST);
        }

        ++parser->index;
        break;
      }

      case s_req_spaces_before_url:
      {
        if (ch == ' ') break;

        if (parser->method == HTTP_CONNECT) {
          UPDATE_STATE(s_req_server_start);
          break;
        }

        if (ch == '/' || ch == '*') {
          parser->url.path.base = p;
          UPDATE_STATE(s_req_path);
          break;
        }

        if (IS_ALPHA(ch)) {
          parser->url.schema.base = p;
          UPDATE_STATE(s_req_schema);
          break;
        }

        RETURN(HTTP_BAD_REQUEST);
      }

      case s_req_schema:
      {
        if (IS_ALPHA(ch)) break;

        if (ch == ':') {
          parser->url.schema.len = p - parser->url.schema.base;
          UPDATE_STATE(s_req_schema_slash);
          break;
        }

        RETURN(HTTP_BAD_REQUEST);
      }

      case s_req_schema_slash:
      {
        if (ch == '/') {
          UPDATE_STATE(s_req_schema_slash_slash);
          break;
        }

        RETURN(HTTP_BAD_REQUEST);
      }

      case s_req_schema_slash_slash:
      {
        if (ch == '/') {
          UPDATE_STATE(s_req_server_start);
          break;
        }

        RETURN(HTTP_BAD_REQUEST);
      }

      case s_req_server_start:
      {
        if (IS_USERINFO_CHAR(ch) || ch == '[' || ch == ']') {
          parser->url.server.base = p;
          UPDATE_STATE(s_req_server);
          break;
        }

        switch (ch) {
          case '/':
            parser->url.path.base = p;
            UPDATE_STATE(s_req_path);
            break;
          case '?':
            UPDATE_STATE(s_req_query_string_start);
            break;
          case '@':
            parser->url.server.base = p;
            parser->found_at = 1;
            UPDATE_STATE(s_req_server);
            break;
          case ' ':
            UPDATE_STATE(s_req_http_start);
            break;
          case CR:
            parser->http_major = 0;
            parser->http_minor = 9;
            UPDATE_STATE(s_req_line_almost_done);
            break;
          case LF:
            parser->http_major = 0;
            parser->http_minor = 9;
            parser->nread = 0;
            UPDATE_STATE(s_start);
            RETURN(HTTP_OK);
          default:
            RETURN(HTTP_BAD_REQUEST);
        }
        break;
      }

      case s_req_server:
      {
        if (IS_USERINFO_CHAR(ch) || ch == '[' || ch == ']') {
          break;
        }

        switch (ch) {
          case '/':
            parser->url.server.len = p - parser->url.server.base;
            parser->url.path.base = p;
            UPDATE_STATE(s_req_path);
            break;
          case '?':
            parser->url.server.len = p - parser->url.server.base;
            UPDATE_STATE(s_req_query_string_start);
            break;
          case '@':
            if (parser->found_at) {
              RETURN(HTTP_BAD_REQUEST);
            }
            parser->found_at = 1;
            break;
          case ' ':
            parser->url.server.len = p - parser->url.server.base;
            UPDATE_STATE(s_req_http_start);
            break;
          case CR:
            parser->url.server.len = p - parser->url.server.base;
            parser->http_major = 0;
            parser->http_minor = 9;
            UPDATE_STATE(s_req_line_almost_done);
            break;
          case LF:
            parser->url.server.len = p - parser->url.server.base;
            parser->http_major = 0;
            parser->http_minor = 9;
            parser->nread = 0;
            UPDATE_STATE(s_start);
            RETURN(HTTP_OK);
          default:
            RETURN(HTTP_BAD_REQUEST);
        }
        break;
      }

      case s_req_path:
      {
        if (IS_URL_CHAR(ch)) break;

        switch (ch) {
          case '?':
            parser->url.path.len = p - parser->url.path.base;
            UPDATE_STATE(s_req_query_string_start);
            break;
          case '#':
            parser->url.path.len = p - parser->url.path.base;
            UPDATE_STATE(s_req_fragment_start);
            break;
          case ' ':
            parser->url.path.len = p - parser->url.path.base;
            UPDATE_STATE(s_req_http_start);
            break;
          case CR:
            parser->url.path.len = p - parser->url.path.base;
            parser->http_major = 0;
            parser->http_minor = 9;
            UPDATE_STATE(s_req_line_almost_done);
            break;
          case LF:
            parser->url.path.len = p - parser->url.path.base;
            parser->http_major = 0;
            parser->http_minor = 9;
            parser->nread = 0;
            UPDATE_STATE(s_start);
            RETURN(HTTP_OK);
          default:
            RETURN(HTTP_BAD_REQUEST);
        }
        break;
      }

      case s_req_query_string_start:
      {
        if (IS_URL_CHAR(ch) || ch == '?') {
          parser->url.query.base = p;
          UPDATE_STATE(s_req_query_string);
          break;
        }

        switch (ch) {
          case '#':
            UPDATE_STATE(s_req_fragment_start);
            break;
          case ' ':
            UPDATE_STATE(s_req_http_start);
            break;
          case CR:
            parser->http_major = 0;
            parser->http_minor = 9;
            UPDATE_STATE(s_req_line_almost_done);
            break;
          case LF:
            parser->http_major = 0;
            parser->http_minor = 9;
            parser->nread = 0;
            UPDATE_STATE(s_start);
            RETURN(HTTP_OK);
          default:
            RETURN(HTTP_BAD_REQUEST);
        }
        break;
      }

      case s_req_query_string:
      {
        if (IS_URL_CHAR(ch) || ch == '?') {
          break;
        }

        switch (ch) {
          case '#':
            parser->url.query.len = p - parser->url.query.base;
            UPDATE_STATE(s_req_fragment_start);
            break;
          case ' ':
            parser->url.query.len = p - parser->url.query.base;
            UPDATE_STATE(s_req_http_start);
            break;
          case CR:
            parser->url.query.len = p - parser->url.query.base;
            parser->http_major = 0;
            parser->http_minor = 9;
            UPDATE_STATE(s_req_line_almost_done);
            break;
          case LF:
            parser->url.query.len = p - parser->url.query.base;
            parser->http_major = 0;
            parser->http_minor = 9;
            parser->nread = 0;
            UPDATE_STATE(s_start);
            RETURN(HTTP_OK);
          default:
            RETURN(HTTP_BAD_REQUEST);
        }
        break;
      }

      case s_req_fragment_start:
      {
        if (IS_URL_CHAR(ch) || ch == '?') {
          parser->url.fragment.base = p;
          UPDATE_STATE(s_req_fragment);
          break;
        }

        if (ch == '#') break;

        switch (ch) {
          case ' ':
            UPDATE_STATE(s_req_http_start);
            break;
          case CR:
            parser->http_major = 0;
            parser->http_minor = 9;
            UPDATE_STATE(s_req_line_almost_done);
            break;
          case LF:
            parser->http_major = 0;
            parser->http_minor = 9;
            parser->nread = 0;
            UPDATE_STATE(s_start);
            RETURN(HTTP_OK);
          default:
            RETURN(HTTP_BAD_REQUEST);
        }
        break;
      }

      case s_req_fragment:
      {
        if (IS_URL_CHAR(ch) || ch == '?' || ch == '#') {
          break;
        }

        switch (ch) {
          case ' ':
            parser->url.fragment.len = p - parser->url.fragment.base;
            UPDATE_STATE(s_req_http_start);
            break;
          case CR:
            parser->url.fragment.len = p - parser->url.fragment.base;
            parser->http_major = 0;
            parser->http_minor = 9;
            UPDATE_STATE(s_req_line_almost_done);
            break;
          case LF:
            parser->url.fragment.len = p - parser->url.fragment.base;
            parser->http_major = 0;
            parser->http_minor = 9;
            parser->nread = 0;
            UPDATE_STATE(s_start);
            RETURN(HTTP_OK);
          default:
            RETURN(HTTP_BAD_REQUEST);
        }
        break;
      }

      case s_req_http_start:
        switch (ch) {
          case 'H':
            UPDATE_STATE(s_req_http_H);
            break;
          case ' ':
            break;
          default:
            RETURN(HTTP_BAD_REQUEST);
        }
        break;

      case s_req_http_H:
        STRICT_CHECK(ch != 'T', HTTP_BAD_REQUEST);
        UPDATE_STATE(s_req_http_HT);
        break;

      case s_req_http_HT:
        STRICT_CHECK(ch != 'T', HTTP_BAD_REQUEST);
        UPDATE_STATE(s_req_http_HTT);
        break;

      case s_req_http_HTT:
        STRICT_CHECK(ch != 'P', HTTP_BAD_REQUEST);
        UPDATE_STATE(s_req_http_HTTP);
        break;

      case s_req_http_HTTP:
        STRICT_CHECK(ch != '/', HTTP_BAD_REQUEST);
        UPDATE_STATE(s_req_first_http_major);
        break;

      /* first digit of major HTTP version */
      case s_req_first_http_major:
        if (UNLIKELY(ch < '1' || ch > '9')) {
          RETURN(HTTP_BAD_REQUEST);
        }

        parser->http_major = ch - '0';
        UPDATE_STATE(s_req_http_major);
        break;

      /* major HTTP version or dot */
      case s_req_http_major:
      {
        if (ch == '.') {
          UPDATE_STATE(s_req_first_http_minor);
          break;
        }

        if (UNLIKELY(!IS_NUM(ch))) {
          RETURN(HTTP_BAD_REQUEST);
        }

        parser->http_major *= 10;
        parser->http_major += ch - '0';
        if (UNLIKELY(parser->http_major > 999)) {
          RETURN(HTTP_BAD_REQUEST);
        }

        break;
      }

      /* first digit of minor HTTP version */
      case s_req_first_http_minor:
        if (UNLIKELY(!IS_NUM(ch))) {
          RETURN(HTTP_BAD_REQUEST);
        }

        parser->http_minor = ch - '0';
        UPDATE_STATE(s_req_http_minor);
        break;

      /* minor HTTP version or end of request line */
      case s_req_http_minor:
      {
        if (ch == CR) {
          UPDATE_STATE(s_req_line_almost_done);
          break;
        }

        if (ch == LF) {
          parser->nread = 0;
          UPDATE_STATE(s_start);
          RETURN(HTTP_OK);
        }

        /* XXX allow spaces after digit? */

        if (UNLIKELY(!IS_NUM(ch))) {
          RETURN(HTTP_BAD_REQUEST);
        }

        parser->http_minor *= 10;
        parser->http_minor += ch - '0';
        if (UNLIKELY(parser->http_minor > 999)) {
          RETURN(HTTP_BAD_REQUEST);
        }

        break;
      }

      /* end of request line */
      case s_req_line_almost_done:
      {
        if (UNLIKELY(ch != LF)) {
          RETURN(HTTP_BAD_REQUEST);
        }

        parser->nread = 0;
        UPDATE_STATE(s_start);
        RETURN(HTTP_OK);
      }

      default:
        assert(0 && "unhandled state");
        RETURN(HTTP_BAD_REQUEST);
    }
  }

  parser->state = CURRENT_STATE();
  parser->last_pos = p;
  return HTTP_AGAIN;
}

int http_parse_headers(http_parser *parser, char *data, char *last) {
  char ch;
  char *p;
  http_buf_t *buf;
  enum state p_state = (enum state) parser->state;

  assert(data <= last);
  p = parser->last_pos ? parser->last_pos : data;
  assert(p >= data || p <= last);

  for (; p < last; p++) {
    ch = *p;
    parser->nread += 1;
    if (UNLIKELY(parser->nread > parser->max_header_line_size)) {
      RETURN(HTTP_BAD_REQUEST);
    }

    switch (CURRENT_STATE()) {
      case s_start:
      {
        if (ch == CR) {
          UPDATE_STATE(s_headers_almost_done);
          break;
        }

        if (ch == LF) {
          parser->nread = 0;
          UPDATE_STATE(s_start);
          RETURN(HTTP_OK);
        }

        if (UNLIKELY(!TOKEN(ch))) {
          RETURN(HTTP_BAD_REQUEST);
        }

        parser->headers[parser->nbuf].base = p;
        ++parser->index;
        UPDATE_STATE(s_header_field);
        break;
      }

      case s_header_field:
      {
        if (ch == ':') {
          buf = &(parser->headers[parser->nbuf]);
          buf->len = p - buf->base;
          ++parser->index;
          UPDATE_STATE(s_header_space_before_value);
          break;
        }

        if (UNLIKELY(!TOKEN(ch))) {
          RETURN(HTTP_BAD_REQUEST);
        }

        break;
      }

      case s_header_space_before_value:
      {
        if (ch == ' ' || ch == '\t') break;

        if (ch == CR) {
          UPDATE_STATE(s_header_almost_done);
          break;
        }

        if (ch == LF) {
          parser->nbuf += 2;
          parser->index = 0;
          parser->nread = 0;
          UPDATE_STATE(s_start);

          if (parser->nbuf == HTTP_MAX_HEADERS_PER_READ * 2) {
            RETURN(HTTP_DONE);
          }

          break;
        }

        parser->headers[parser->nbuf + 1].base = p;
        ++parser->index;
        UPDATE_STATE(s_header_value);
        break;
      }

      case s_header_value:
      {
        if (ch == ' ' || ch == '\t') {
          UPDATE_STATE(s_header_space_after_value);
          break;
        }

        if (ch == CR) {
          buf = &(parser->headers[parser->nbuf + 1]);
          buf->len = p - buf->base;
          UPDATE_STATE(s_header_almost_done);
          break;
        }

        if (ch == LF) {
          buf = &(parser->headers[parser->nbuf + 1]);
          buf->len = p - buf->base;
          parser->nbuf += 2;
          parser->index = 0;
          parser->nread = 0;
          UPDATE_STATE(s_start);

          if (parser->nbuf == HTTP_MAX_HEADERS_PER_READ * 2) {
            RETURN(HTTP_DONE);
          }

          break;
        }

        break;
      }

      case s_header_space_after_value:
      {
        if (ch == ' ' || ch == '\t') {
          break;
        }

        if (ch == CR) {
          buf = &(parser->headers[parser->nbuf + 1]);
          buf->len = p - buf->base;
          UPDATE_STATE(s_header_almost_done);
          break;
        }

        if (ch == LF) {
          buf = &(parser->headers[parser->nbuf + 1]);
          buf->len = p - buf->base;
          parser->nbuf += 2;
          parser->index = 0;
          parser->nread = 0;
          UPDATE_STATE(s_start);

          if (parser->nbuf == HTTP_MAX_HEADERS_PER_READ * 2) {
            RETURN(HTTP_DONE);
          }

          break;
        }

        UPDATE_STATE(s_header_value);
        break;
      }

      case s_header_almost_done:
      {
        STRICT_CHECK(ch != LF, HTTP_BAD_REQUEST);

        parser->nbuf += 2;
        parser->index = 0;
        parser->nread = 0;
        UPDATE_STATE(s_start);

        if (parser->nbuf == HTTP_MAX_HEADERS_PER_READ * 2) {
          RETURN(HTTP_DONE);
        }

        break;
      }

      case s_headers_almost_done:
        STRICT_CHECK(ch != LF, HTTP_BAD_REQUEST);
        parser->nread = 0;
        UPDATE_STATE(s_start);
        RETURN(HTTP_OK);

      default:
        assert(0 && "unhandled state");
        RETURN(HTTP_BAD_REQUEST);
    }
  }

  parser->state = CURRENT_STATE();
  parser->last_pos = p;
  return HTTP_AGAIN;
}

int http_parse_host(http_url *url, char *data, size_t len, uint8_t found_at) {
  char ch;
  char *p;
  enum http_host_state s = found_at ? s_http_userinfo_start : s_http_host_start;

  char *end = data + len;
  for (p = data; p < end; p++) {
    ch = *p;

    switch (s) {
      case s_http_userinfo_start:
      {
        if (IS_USERINFO_CHAR(ch)) {
          url->userinfo.base = p;
          ++url->userinfo.len;
          s =  s_http_userinfo;
          break;
        }

        if (ch == '@') {
          s = s_http_host_start;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_userinfo:
      {
        if (IS_USERINFO_CHAR(ch)) {
          ++url->userinfo.len;
          break;
        }

        if (ch == '@') {
          s = s_http_host_start;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_host_start:
      {
        if (IS_HOST_CHAR(ch)) {
          url->host.base = p;
          ++url->host.len;
          s = s_http_host;
        }

        if (ch == '[') {
          s = s_http_host_v6_start;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_host:
      {
        if (IS_HOST_CHAR(ch)) {
          ++url->host.len;
          break;
        }

        if (ch == ':') {
          s = s_http_host_port_start;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_host_v6_start:
      {
        if (IS_HEX(ch) || ch == ':' || ch == '.') {
          url->host.base = p;
          ++url->host.len;
          s =  s_http_host_v6;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_host_v6:
      {
        if (IS_HEX(ch) || ch == ':' || ch == '.') {
          ++url->host.len;
          break;
        }

        if (ch == ']') {
          s = s_http_host_v6_end;
          break;
        }

        if (ch == '%') {
          s = s_http_host_v6_zone_start;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_host_v6_zone_start:
      {
        /* RFC 6874 Zone ID consists of 1*( unreserved / pct-encoded) */
        if (IS_ALPHANUM(ch) || ch == '%' || ch == '.' || ch == '-' || ch == '_' ||
            ch == '~') {
          ++url->host.len;
          s = s_http_host_v6_zone;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_host_v6_zone:
      {

        if (IS_ALPHANUM(ch) || ch == '%' || ch == '.' || ch == '-' || ch == '_' ||
            ch == '~') {
          ++url->host.len;
          break;
        }

        if (ch == ']') {
          s = s_http_host_v6_end;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_host_v6_end:
      {
        if (ch == ':') {
          s = s_http_host_port_start;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_host_port_start:
      {
        if (IS_NUM(ch)) {
          url->port.base = p;
          ++url->port.len;
          s =  s_http_host_port;
          break;
        }

        return HTTP_ERROR;
      }

      case s_http_host_port:
      {
        if (IS_NUM(ch)) {
          ++url->port.len;
          break;
        }

        return HTTP_ERROR;
      }

      default:
        assert(0 && "unhandled state");
        return HTTP_ERROR;
    }
  }

  return HTTP_OK;
}

int http_parse_url(http_url *url, char *data, size_t len) {
  char ch;
  char *p;
  uint8_t found_at = 0;
  enum state s = s_start;
  memset(url, 0, sizeof(*url));

  char *end = data + len;
  for (p = data; p < end; p++) {
    ch = *p;

    switch (s) {
      case s_start:
      {
        if (ch == '/') {
          url->path.base = p;
          ++url->path.len;
          s = s_req_path;
          break;
        }

        if (IS_ALPHA(ch)) {
          url->schema.base = p;
          ++url->schema.len;
          s = s_req_schema;
          break;
        }

        return HTTP_ERROR;
      }

      case s_req_schema:
      {
        if (IS_ALPHA(ch)) {
          ++url->schema.len;
          break;
        }

        if (ch == ':') {
          s = s_req_schema_slash;
          break;
        }

        return HTTP_ERROR;
      }

      case s_req_schema_slash:
      {
        if (ch == '/') {
          s = s_req_schema_slash_slash;
          break;
        }

        return HTTP_ERROR;
      }

      case s_req_schema_slash_slash:
      {
        if (ch == '/') {
          s = s_req_server_start;
          break;
        }

        return HTTP_ERROR;
      }

      case s_req_server_start:
      {
        if (IS_USERINFO_CHAR(ch) || ch == '[' || ch == ']') {
          url->server.base = p;
          ++url->server.len;
          s = s_req_server;
          break;
        }

        switch (ch) {
          case '/':
            url->path.base = p;
            ++url->path.len;
            s = s_req_path;
            break;
          case '?':
            s = s_req_query_string_start;
            break;
          case '@':
            url->server.base = p;
            ++url->server.len;
            found_at = 1;
            s = s_req_server;
            break;
          default:
            return HTTP_ERROR;
        }
        break;
      }

      case s_req_server:
      {
        if (IS_USERINFO_CHAR(ch) || ch == '[' || ch == ']') {
          ++url->server.len;
          break;
        }

        switch (ch) {
          case '/':
            url->path.base = p;
            ++url->path.len;
            s = s_req_path;
            break;
          case '?':
            s = s_req_query_string_start;
            break;
          case '@':
            if (found_at) return HTTP_ERROR;
            found_at = 1;
            break;
          default:
            return HTTP_ERROR;
        }
        break;
      }

      case s_req_path:
      {
        if (IS_URL_CHAR(ch)) {
          ++url->path.len;
          break;
        }

        switch (ch) {
          case '?':
            s = s_req_query_string_start;
            break;
          case '#':
            s = s_req_fragment_start;
            break;
          default:
            return HTTP_ERROR;
        }
        break;
      }

      case s_req_query_string_start:
      {
        if (IS_URL_CHAR(ch) || ch == '?') {
          url->query.base = p;
          ++url->query.len;
          s = s_req_query_string;
          break;
        }

        if (ch == '#') {
          s = s_req_fragment_start;
          break;
        }
        
        return HTTP_ERROR;
      }

      case s_req_query_string:
      {
        if (IS_URL_CHAR(ch) || ch == '?') {
          ++url->query.len;
          break;
        }

        if (ch == '#') {
          s = s_req_fragment_start;
          break;
        }
        
        return HTTP_ERROR;
      }

      case s_req_fragment_start:
      {
        if (IS_URL_CHAR(ch) || ch == '?') {
          url->fragment.base = p;
          ++url->fragment.len;
          s = s_req_fragment;
          break;
        }

        if (ch == '#') break;

        return HTTP_ERROR;
      }

      case s_req_fragment:
      {
        if (IS_URL_CHAR(ch) || ch == '?' || ch == '#') {
          ++url->fragment.len;
          break;
        }

        return HTTP_ERROR;
      }

      default:
        assert(0 && "unhandled state");
        return HTTP_ERROR;
    }
  }

  char *server = url->server.base;
  size_t length = url->server.len;
  if (server && len > 0) {
    return http_parse_host(url, server, length, found_at);
  }

  return HTTP_OK;
}
