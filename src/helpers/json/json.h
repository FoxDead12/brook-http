//
//  json.h
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#ifndef json_h
#define json_h

#include <stdio.h>
#include <json-c/json.h>
#include "../types/types.h"

int json_get_int (json_object* obj, char* name, int def);
http_str_s json_get_str (json_object* obj, char* name, http_str_s def);


#endif /* json_h */
