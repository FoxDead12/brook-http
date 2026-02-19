#ifndef _BROOK_HTTP_REQUEST_H
#define  _BROOK_HTTP_REQUEST_H

#include "core/config.h"

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
