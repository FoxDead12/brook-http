//
//  request.c
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#include "request.h"

int request_set_headers (http_connection_struct* con) {
        
    // validate first line of header if is valid
    regmatch_t matches[4];
    int reg = regexec(&con->worker->server->regex_header, con->b_header.start, 4, matches, 0);
    
    if (reg == REG_NOMATCH) {
        return HTTP_ERROR;
    }
    
    con->request.method.data   = con->b_header.start + matches[1].rm_so;
    con->request.method.length = matches[1].rm_eo - matches[1].rm_so;

    con->request.url.data   = con->b_header.start + matches[2].rm_so;
    con->request.url.length = matches[2].rm_eo - matches[2].rm_so;
    
    if (matches[3].rm_so >= 0) {
        con->request.params.data   = con->b_header.start + matches[3].rm_so + 1;
        con->request.params.length = matches[3].rm_eo - (matches[3].rm_so + 1);
    }
    
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
