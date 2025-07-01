//
//  json.c
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#include "json.h"

int json_get_int (json_object* obj, char* name, int def) {
    
    json_object *number = NULL;
    
    if (json_object_object_get_ex(obj, name, &number)) {
        return json_object_get_int(number);
    }

    return def;
}
