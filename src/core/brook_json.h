//
//  brook_json.h
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#ifndef brook_json_h
#define brook_json_h

#include "brook_core.h"

json_object* json_parse (char* data);
int json_get_int (const char* key, json_object* o, int def);
brook_array_t* json_get_array(const char* key, json_object* o);
#endif /* brook_json_h */
