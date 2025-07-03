//
//  request.c
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#include "request.h"

int set_headers_of_request (http_request_struct *client) {
    
    client->header.method = parse_method_of_header(client->header.data.start);
    
    client->header.content_type = parse_value_of_header(client->header.data.start, "Content-Type");
    client->header.host = parse_value_of_header(client->header.data.start, "Host");
    client->header.connection = parse_value_of_header(client->header.data.start, "Connection");
    client->header.content_length = conv_str_to_int(parse_value_of_header(client->header.data.start, "Content-Length"));
        
    return 0;
}

http_str_s parse_method_of_header (char* buffer) {
    
    http_str_s s;
    s.length = 0;
    s.data = 0;

    char* method = strchr(buffer, ' ');
    
    s.data = buffer;
    s.length = (int)(method - buffer);

    return s;
}

http_str_s parse_value_of_header (char* buffer, const char* header_name) {
    
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


int handle_request (http_request_struct* cleint) {
    
    // TODO: valdiate route gatekeeper
    // TODO: validate user has acess
    
    return 0;
}
