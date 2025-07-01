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

int json_get_int (json_object* obj, char* name, int def);

#endif /* json_h */
