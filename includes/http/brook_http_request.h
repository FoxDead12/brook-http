#ifndef _BROOK_HTTP_REQUEST_H
#define  _BROOK_HTTP_REQUEST_H

#include "core/config.h"

#define T(v) (1 << ((v) & 7))
#define BIT_AT(a, i)                                                \
  (!!((unsigned int) (a)[(unsigned int) (i) >> 3] &                  \
  (1 << ((unsigned int) (i) & 7))))

  // ... to allow UTF8 + ASCII bytes
// #define IS_URL_CHAR(c)                                                         \
//   (BIT_AT(normal_url_char, (unsigned char)c) || ((c) & 0x80))

// ... to only allow ASCII bytes
#define IS_URL_CHAR(c)      (BIT_AT(normal_url_char, (unsigned char)c))
#define IS_NUM(c)           ((c) >= '0' && (c) <= '9')

#define CONTENT_LENGTH "content-length"
#define CONTENT_TYPE "content-type"

#define MAX_BODY_SIZE 1048576 // 1 MB

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

  s_req_header_field_start,
  s_req_header_field,
  s_req_header_value_start,
  s_req_header_value,
  s_req_header_value_done,

  s_req_headers_done,
  s_req_body,
  s_req_done
};

enum header_state {
  s_general,
  s_C,
  s_CO,
  s_CON,
  s_CONTENT,
  s_content_length,
  s_content_type
};

int brook_http_parse(brook_http_parse_t* parser, unsigned char* data, size_t len);

#endif
