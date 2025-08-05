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
    
    // ... Validate and parse first line of socket message
    regmatch_t matches[4];
    if (regexec(&connection->conf->regex.http_line, (char*) connection->buff->buf->start, 4, matches, 0) == REG_NOMATCH) {
        return BROOK_ERROR;
    }
    
    // ... Parse method of regex
    connection->http->method.data = connection->buff->buf->start + matches[0].rm_so;
    connection->http->method.len  = matches[1].rm_eo - matches[1].rm_so;
    
    // ... Parse url withou params of regex
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
    
    if (brook_array_find_value(connection->conf->http.allow_content_types, connection->http->header.content_type) == -1) {
        return BROOK_ERROR;
    }
        
    if ( brook_strncmp(connection->http->method.data, "POST", connection->http->method.len) != 0 && brook_strncmp(connection->http->method.data, "PATCH", connection->http->method.len) != 0 ) {
        return BROOK_OK;
    }
    
    if ( brook_strncmp(connection->http->header.content_type.data, "application/vnd.api+json", connection->http->header.content_type.len) == 0 || brook_strncmp(connection->http->header.content_type.data, "application/json", connection->http->header.content_type.len) == 0 ) {
        return brook_http_json_request(connection);
    }
    
    int content_length = connection->http->header.content_length;
    
    // ... check content length necessary
    if (content_length <= 0) return BROOK_OK;
    if (content_length > connection->conf->http.max_body_size) return BROOK_ERROR;
    
    // ... calculate current lenght loaded
    size_t header_size = end - connection->buff->buf->start;
    size_t body_already_loaded = connection->buff->buf->len - header_size;
    
    // ... check if already load all body is done
    if (body_already_loaded >= content_length) {
        return BROOK_OK; // request already readed, and transform data to json
    }
    
    // ... need continue reading socket, but if json object create buffer with body lenght, if file will reuse same buffer
    
    brook_chain_t* b = malloc(sizeof(brook_chain_t));
    b->buf = malloc(sizeof(brook_buffer_t));
    b->buf->start = calloc(1, connection->conf->http.buffers_size + 1);
    b->buf->pos   = b->buf->start;
    b->buf->size  = connection->conf->http.buffers_size;
    
    memmove(b->buf->start, end, body_already_loaded);
    
    b->next = connection->buff;
    connection->buff = b;
    
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

int
brook_http_json_request (brook_connection_t* connection) {
    return BROOK_OK;
}
