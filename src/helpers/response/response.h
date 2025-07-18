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


int          send_json_api_response_error (http_connection_struct* con, int status, const char* code, const char* detail);
int          send_json_api_response (http_connection_struct* con);
size_t       response_header_format (char** r, int status, size_t content_lenght);
json_object* build_json_api_error_obj (int status, const char* code, const char* detail);
json_object* build_json_api_error_obj_from_db_result (PGresult* res);

#endif /* response_h */
