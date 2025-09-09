//
//  brook_http_parse.h
//  http-c-broker
//
//  Created by David Xavier on 02/08/2025.
//

#ifndef brook_http_parse_h
#define brook_http_parse_h

#include "brook_http.h"

int brook_http_parse(brook_connection_t* connection);
int brook_http_header_parse(brook_connection_t* connection);
int brook_http_body_parse(brook_connection_t* connection);
int brook_http_set_method (brook_connection_t* connection, brook_str_t method);
brook_str_t brook_http_request_header_value (char* buf, const char* key);


#endif /* brook_http_parse_h */
