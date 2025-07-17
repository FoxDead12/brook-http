//
//  response.c
//  http-c-broker
//
//  Created by David Xavier on 17/07/2025.
//

#include "response.h"

int send_json_api_response (http_connection_struct* con) {
    
    const char* b = json_object_to_json_string(con->response.json_api.b);
    size_t size   = strlen(b);
    
    char* response;
    size_t response_len = response_header_format(&response, 200, size);
    write(con->socket, response, response_len);
    
    write(con->socket, b, size);
    
    free(response);
    json_object_put(con->response.json_api.b);
    
    return HTTP_OK;
}

size_t response_header_format (char** r, int status, size_t content_lenght) {
    
    const char *response =
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: Content-Type: application/vnd.api+json;\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n";

    size_t len = asprintf(r, response, status, content_lenght);
    
    return len;
}
