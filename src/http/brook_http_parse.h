//
//  brook_http_parse.h
//  http-c-broker
//
//  Created by David Xavier on 02/08/2025.
//

#ifndef brook_http_parse_h
#define brook_http_parse_h

#include "brook_http.h"


typedef struct brook_config_s brook_config_t;
typedef struct brook_chain_s brook_chain_t;

int brook_http_parse (brook_connection_t* connection);

#endif /* brook_http_parse_h */
