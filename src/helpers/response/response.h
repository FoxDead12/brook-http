//
//  response.h
//  http-c-broker
//
//  Created by David Xavier on 17/07/2025.
//

#ifndef response_h
#define response_h

#include <stdio.h>
#include "../types/types.h"

int    send_json_api_response (http_connection_struct* con);
size_t response_header_format (char** r, int status, size_t content_lenght);

#endif /* response_h */
