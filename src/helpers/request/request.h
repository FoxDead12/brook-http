//
//  request.h
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#ifndef request_h
#define request_h

#include <stdio.h>
#include <string.h>
#include "../types/types.h"

http_str_s parse_method_of_header (char* buffer);
http_str_s parse_value_of_header (char* buffer, const char* header_name);
int set_headers_of_request (http_request_struct *client);

#endif /* request_h */
