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
        send_json_api_response_error(con, 400, "HTTP_BROKER_ERROR_HEADER", "Header invalid format");
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
    
    if (con->request.headers.content_length > 0 && comp_str_to_str(con->request.headers.content_type, http_str("application/json")) == 1) {
        send_json_api_response_error(con, 400, "HTTP_BROKER_ERROR_HEADER", "Header content-type invalid only allow application json");
        return HTTP_ERROR;
    }
    
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

int request_body_transform_to_json (http_connection_struct* con) {
    
    char* b = calloc(1, con->c_body.bytes);
    size_t n = 0;
    
    http_buffer_s* d = con->c_body.first;
    
    while (1) {

        if (d == NULL) {
            break;
        }

        memmove(b + n, d->start, d->length);
        n += d->length;
        
        http_buffer_s* s = d;
        d = d->next;
        
        if (s->need_free == 0) {
            free(s->start);
        }
        
        free(s);
    }
    
    con->request.b = json_tokener_parse(b);
    free(b);
    
    return request_body_validate_json(con);
    
}

int request_body_validate_json (http_connection_struct* con) {
    
    json_object* data;
    if (json_object_object_get_ex(con->request.b, "data", &data)) {
        
        json_object* type;
        if (!json_object_object_get_ex(data, "type", &type)) {
            send_json_api_response_error(con, 400, "HTTP_BROKER_ERROR_BODY", "Need indicate parameter 'type' in body");
            return HTTP_ERROR;
        }
        
        if (comp_str_to_str(con->request.method, http_str("PATCH")) == 0) {
            json_object* id;
            if (!json_object_object_get_ex(data, "id", &id)) {
                send_json_api_response_error(con, 400, "HTTP_BROKER_ERROR_BODY", "Need indicate parameter 'id' in body");
                return HTTP_ERROR;
            }
            con->request.id = id;
        }
        
        json_object* attributes;
        if (!json_object_object_get_ex(data, "attributes", &attributes)) {
            send_json_api_response_error(con, 400, "HTTP_BROKER_ERROR_BODY", "Need indicate parameter 'attributes' in body");
            return HTTP_ERROR;
        }
        con->request.attributes = attributes;

    } else {
        send_json_api_response_error(con, 400, "HTTP_BROKER_ERROR_BODY", "Need indicate parameter 'data' in body");
        return HTTP_ERROR;
    }
    
    return HTTP_OK;
}
