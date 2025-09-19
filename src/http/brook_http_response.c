//
//  brook_http_response.c
//  http-c-broker
//
//  Created by David Xavier on 09/09/2025.
//

#include "brook_http_response.h"
#include "brook_http.h"
#include "../core/brook_core.h"

int
brook_http_response_send (brook_connection_t* connection, int status, u_char* body, size_t body_len) {

    brook_http_response_t* response = connection->http->response;

    // ... set state of connection to write in next event ... //
    connection->state = WRITING_SOCKET_MESSAGE;

    response->response_status = status;
    response->response_status_description = brook_http_response_status_description(status);

    if (connection->http->header.content_type.data == NULL) {
        response->response_content_type = (brook_str_t) brook_string("application/vnd.api+json");
    } else {
        response->response_content_type.data = connection->http->header.content_type.data; // I MAKE THIS BECAUSE FOR NOW WE ONLY ASSUME 2 TYPES OF CONTENT
        response->response_content_type.len = connection->http->header.content_type.len;
    }

    const char *template =
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %.*s\r\n"
        "Content-Length: %zu\r\n"
        "Server: brook\r\n"
        "connection: keep-alive\r\n"
        "\r\n"
		"%.*s";

	response->response_len = asprintf((char**) &response->response, template, status, response->response_status_description.data, response->response_content_type.len, response->response_content_type.data, body_len, body_len, body);

	response->response_len_sended = 0;

    return BROOK_OK;
}

brook_str_t
brook_http_response_status_description (int status_number) {

    if (status_number == 200) {
        return (brook_str_t) brook_string("OK");
    } else if (status_number == 201) {
        return (brook_str_t) brook_string("Created");
    } else if (status_number == 400) {
        return (brook_str_t) brook_string("Bad Request");
    } else if (status_number == 404) {
        return (brook_str_t) brook_string("Not Found");
    };

    return (brook_str_t) brook_string("");
}
