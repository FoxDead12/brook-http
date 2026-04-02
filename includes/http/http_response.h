#ifndef _BROOK_HTTP_RESPONSE_H
#define  _BROOK_HTTP_RESPONSE_H

#include "core/config.h"

int brook_http_response_static(brook_connection_t* con, uint16_t code, brook_str_t message, brook_str_t detail);
int brook_http_response_add_status(brook_connection_t* con, uint16_t status);
int brook_http_response_add_header(brook_connection_t* con, brook_str_t data);
int brook_http_response_add_header_json(brook_connection_t* con, brook_str_t key, brook_str_t data);
int brook_http_response_add_content_length(brook_connection_t* con, uint64_t len);
int brook_http_response_add_body(brook_connection_t* con, brook_str_t data);
int brook_http_response_buffer_join(brook_connection_t* con, brook_str_t data);
static const char* brook_http_status_code_str(uint16_t code);

#endif
