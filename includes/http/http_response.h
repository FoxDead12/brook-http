#ifndef _BROOK_HTTP_RESPONSE_H
#define  _BROOK_HTTP_RESPONSE_H

#include "core/config.h"

int brook_http_response_static(brook_connection_t* con, uint16_t code, brook_str_t message, brook_str_t detail);
static const char* brook_http_status_code_str(uint16_t code);

int brook_http_response_add_header(brook_connection_t* con, brook_str_t data);
int brook_http_response_add_body(brook_connection_t* con, brook_str_t data);
int brook_http_response_buffer_join(brook_connection_t* con, brook_str_t data);

#endif
