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
    
    int rs;
    
    switch (state) {
        case READING_HEADER:
            rs = brook_http_header_handler(connection);
            break;
        case READING_BODY:
            break;
    }
    
    return rs;
}

int
brook_http_header_handler (brook_connection_t* connection) {
    
    regmatch_t matches[4];
    if (regexec(&connection->conf->regex.http_line, (char*) connection->buff->buf->start, 4, matches, 0) == REG_NOMATCH) {
        return BROOK_ERROR;
    }
    
    // Parse method of regex
    connection->http->method.data = connection->buff->buf->start + matches[0].rm_so;
    connection->http->method.len  = matches[1].rm_eo - matches[1].rm_so;
    
    // Parse url withou params of regex
    connection->http->url.data = connection->buff->buf->start + matches[2].rm_so;
    connection->http->url.len  = matches[2].rm_eo - matches[2].rm_so;
    
    // Parse params of url
    if (matches[3].rm_so >= 0) {
        connection->http->params.data = connection->buff->buf->start + matches[3].rm_so + 1;
        connection->http->params.len  = matches[3].rm_eo - (matches[3].rm_so + 1);
    }
    
    connection->buff->buf->end = connection->buff->buf->start + connection->buff->buf->len;
    
    u_char* end = (u_char*) strstr((char*) connection->buff->buf->start, "\r\n\r\n");
    if (end == NULL) {
        return BROOK_ERROR;
    }
    end += 4; // jump '\r\n\r\n'
    
    connection->http->header.connection     = brook_http_request_header_value((char*) connection->buff->buf->start, "connection");
    connection->http->header.host           = brook_http_request_header_value((char*) connection->buff->buf->start, "host");
    connection->http->header.content_type   = brook_http_request_header_value((char*) connection->buff->buf->start, "content-type");
    connection->http->header.content_length = brook_str_to_int(brook_http_request_header_value((char*) connection->buff->buf->start, "content-length"));
    
    // TODO: now need check if exist content lenght
    int content_length = connection->http->header.content_length;
    
    if (content_length > 0) {
        
        if (content_length > connection->conf->http.max_body_size) {
            return BROOK_ERROR;
        }
        
        // check if my content lenght fit in current buffer
        size_t header_size = end - connection->buff->buf->start;
        size_t body_already_loaded = connection->buff->buf->len - header_size;
        
        if (body_already_loaded >= content_length) {
            return BROOK_OK; // request already readed, and transform data to json
        } else {
            // need create buffer with size of
        }
        
    }
    
    return BROOK_OK;
}

brook_str_t
brook_http_request_header_value (char* buf, const char* key) {
    
    brook_str_t s = {0, NULL};
    
    
    size_t key_len = strlen(key);
    char* pos = (char*)buf;

    while ((pos = strcasestr(pos, key)) != NULL) {
        if ((pos == buf || *(pos - 1) == '\n') && strncasecmp(pos, key, key_len) == 0) {
            
            char* start = strchr(pos, ':');
            if (start == NULL) {
                return s;
            }
            
            start += 1; // jump ':'
            while (*start == ' ' || *start == '\t') start++;
            
            char* end = strstr(start, "\r\n");
            if (end == NULL) {
                return s;
            }
            
            s.data = (u_char*) start;
            s.len  = end - start;
            return s;
            
        }
        pos++;
    }

    return s;
}
