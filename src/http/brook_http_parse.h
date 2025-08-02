//
//  brook_http_parse.h
//  http-c-broker
//
//  Created by David Xavier on 02/08/2025.
//

#ifndef brook_http_parse_h
#define brook_http_parse_h

#include "brook_http.h"

typedef struct brook_connection_s brook_connection_t;

int brook_http_parse(brook_connection_t* connection);
int brook_http_header_handler(brook_connection_t* connection);

#endif /* brook_http_parse_h */
