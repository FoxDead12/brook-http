//
//  request.c
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#include "request.h"

int request_set_headers (http_connection_struct* con) {
        
    con->request.method = parse_method_of_header(con->b_header.start);
    con->request.url    = parse_url_of_header(con->b_header.start);
    
    con->request.headers.host = request_parse_header_str(con->b_header.start, "Host");
    con->request.headers.connection = request_parse_header_str(con->b_header.start, "Connection");
    con->request.headers.content_type = request_parse_header_str(con->b_header.start, "Content-Type");
    con->request.headers.content_length = str_to_int(request_parse_header_str(con->b_header.start, "Content-Length"));
    
    return HTTP_OK;
}

http_str_s request_parse_header_str (char* buffer, const char* header_name) {
    
    http_str_s s;
    s.length = 0;
    s.data = NULL;
    
    char* h = strcasestr(buffer, header_name);
    
    if (h == NULL) {
        return s;
    }
    
    char *p = strchr(h, ':');
    
    if (p == NULL) {
        return s;
    }

    // jump two dots ':'
    p += 1;
    
    // jump white spaces
    while (*p == ' ' || *p == '\t') p++;
    
    char* value = p;
    char* value_end = strchr(value, '\r');
    
    s.length = (int)(value_end - value);
    s.data = value;
    
    return s;
    
}

http_str_s parse_method_of_header (char* buffer) {
    
    http_str_s s;
    s.length = 0;
    s.data = NULL;

    char* method = strchr(buffer, ' ');
    
    s.data = buffer;
    s.length = (int)(method - buffer);

    return s;
}

http_str_s parse_url_of_header (char* buffer) {
    
    http_str_s s;
    s.length = 0;
    s.data = NULL;
        
    char* url = strchr(buffer, ' ');
    url += 1;
    
    char* end = url;
    
    while (*end != ' ') {
        end += 1;
    }
    
    char* end_line = strchr(buffer, '\r');
    
    if (end >= end_line) {
        return s;
    }

    s.length = end - url;
    s.data = url;
    
    return s;
}
