//
//  brook_http.h
//  http-c-broker
//
//  Created by David Xavier on 01/08/2025.
//

#ifndef brook_http_h
#define brook_http_h

#include "brook_config.h"
#include "brook_http_parse.h"
#include "brook_json_api.h"

typedef struct brook_http_s brook_http_t;
typedef struct brook_http_header_s brook_http_header_t;
typedef enum   brook_http_status_s brook_http_status_t;

enum brook_http_status_s {
    READING_HEADER,
    READING_BODY
};

struct brook_http_header_s {
    brook_str_t     host;
    brook_str_t     connection;
    brook_str_t     content_type;
    int             content_length;
};

struct brook_http_s {
    brook_http_header_t header;
    brook_http_status_t state;
    brook_str_t         method;
    brook_str_t         url;
    brook_str_t         params;
	
    brook_json_api_t*   json_api;
    
	brook_buffer_t*		buff_body;      // only pointer to buff of connection
	brook_buffer_t*		buff_header;    // only pointer to buff of connection
};

#endif /* brook_http_h */
