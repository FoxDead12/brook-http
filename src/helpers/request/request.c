//
//  request.c
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#include "request.h"

int request_set_headers (http_connection_struct* con) {
        
    // validate first line of header if is valid
    int reg = regexec(&con->worker->server->regex_header, con->b_header.start, 0, NULL, 0);
    if (reg == REG_NOMATCH) {
        return HTTP_ERROR;
    }
        
    con->request.method = parse_method_of_header(con->b_header.start);
    con->request.url    = parse_url_of_header(con->b_header.start);
    con->request.params = parse_url_params_of_header(con->b_header.start);
    
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
     
    char* line_end = strchr(buffer, '\r');
    char* url = strchr(buffer, ' ');
    
    if (url == NULL) {
        return s;
    }
    
    url += 1;
    
    if (url >= line_end) {
        return s; // invalid position, pointer is not in first row of header
    }
    
    if (*url != '/') {
        return s; // invalid, dont start with '/' the url
    }
    
    char* end = url;
    
    while (*end != ' ' && *end != '?') {
        end += 1;
    }
    
    if (end >= line_end) {
        return s;
    }

    s.length = end - url;
    s.data   = url;
    
    return s;
}

http_str_s parse_url_params_of_header (char* buffer) {
    
    http_str_s s;
    s.length = 0;
    s.data = NULL;
    
    char* line_end = strchr(buffer, '\r');
    
    char* params = strchr(buffer, '?');
    if (params == NULL) {
        return s;
    }
    
    params += 1;
    
    if (params >= line_end) {
        return s; // invalid position, pointer is not in first row of header
    }
    
    char* end = params;
    
    while (*end != ' ') {
        end += 1;
    }
    
    s.length = end - params;
    s.data   = params;
    
    return s;
}
