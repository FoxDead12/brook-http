#ifndef _BROOK_HTTP_REQUEST_H
#define  _BROOK_HTTP_REQUEST_H

#include "core/config.h"

#define T(v) (1 << ((v) & 7))
# define BIT_AT(a, i)                                                \
  (!!((unsigned int) (a)[(unsigned int) (i) >> 3] &                  \
  (1 << ((unsigned int) (i) & 7))))

  // ... to allow UTF8 + ASCII bytes
// #define IS_URL_CHAR(c)                                                         \
//   (BIT_AT(normal_url_char, (unsigned char)c) || ((c) & 0x80))

// ... to only allow ASCII bytes
#define IS_URL_CHAR(c)      (BIT_AT(normal_url_char, (unsigned char)c))


typedef enum {
  DELETE,
  GET,
  POST,
  PUT
} brook_method_e;

enum state {

  s_req_start,
  s_req_method,
  s_req_url,
  s_req_minor,

  s_req_header_field,
  s_req_header_value,

  s_headers_done
};

int brook_http_parse(brook_http_parse_t* parser, unsigned char* data, size_t len);

#endif
