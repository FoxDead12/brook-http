//
//  brook_http_parse.c
//  http-c-broker
//
//  Created by David Xavier on 02/08/2025.
//

#include "brook_http_parse.h"
#include "brook_http.h"
#include "../core/brook_core.h"

int
brook_http_parse (brook_connection_t* connection) {
    
    brook_buffer_t* buf = connection->buff;
    brook_http_status_t state = connection->state;
    
    switch (state) {
        case READING_HEADER:
            brook_http_header_handler(connection);
            break;
        case READING_BODY:
            break;
    }
    
    
    return BROOK_OK;
}

int
brook_http_header_handler (brook_connection_t* connection) {
    
    
    
    
    return BROOK_OK;
}
