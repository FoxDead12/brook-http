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

int         request_set_headers      (http_connection_struct* con);
http_str_s  request_parse_header_str (char* buffer, const char* header_name);
http_str_s  parse_method_of_header   (char* buffer);
http_str_s  parse_url_of_header      (char* buffer);

#endif /* request_h */
