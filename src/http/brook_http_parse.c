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
    
    regmatch_t matches[4];
    if (regexec(&connection->conf->regex.http_line, connection->buff->buf->start, 4, matches, 0) == REG_NOMATCH) {
        return BROOK_ERROR;
    }
    
    // Parse method of regex
    connection->http->method.data = connection->buff->buf->start + matches[0].rm_so;
    connection->http->method.len  = matches[1].rm_eo - matches[1].rm_so;
    
    // Parse url withou params of regex
    connection->http->url.data = connection->buff->buf->start + matches[2].rm_so;
    connection->http->url.len  = matches[2].rm_eo - matches[2].rm_so;
    
    if (matches[3].rm_so >= 0) {
        connection->http->params.data = connection->buff->buf->start + matches[3].rm_so + 1;
        connection->http->params.len  = matches[3].rm_eo - (matches[3].rm_so + 1);
    }
    
    printf("Metodo: %.*s\n", connection->http->method.len, connection->http->method.data);
    printf("Url: %.*s\n", connection->http->url.len, connection->http->url.data);
    printf("Params: %.*s\n", connection->http->params.len, connection->http->params.data);

    
    return BROOK_OK;
}
