//
//  brook_http_response.h
//  http-c-broker
//
//  Created by David Xavier on 09/09/2025.
//

#ifndef brook_http_response_h
#define brook_http_response_h

#include "brook_http.h"

typedef struct brook_http_response_s brook_http_response_t;
typedef enum brook_http_response_state_e brook_http_response_state_t;


struct brook_http_response_s {

    int         response_status;
    brook_str_t response_status_description;
    brook_str_t response_content_type;

    u_char*     header;
    size_t      header_len;

    u_char*     body;
    size_t      body_len;

	size_t      header_data_sended;
    size_t      body_data_sended;
    int         sending_body; // 0 - false ; 1 - true
};


int brook_http_response_send(brook_connection_t* connection, int status, u_char* body, size_t body_len);

brook_str_t brook_http_response_status_description(int status_number);
#endif /* brook_http_response_h */
