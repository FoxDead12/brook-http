//
//  json.c
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#include "json.h"

int json_get_int (json_object* obj, char* name, int def) {
    
    json_object* number = NULL;
    
    if (json_object_object_get_ex(obj, name, &number)) {
        return json_object_get_int(number);
    }

    return def;
}

http_str_s json_get_str (json_object* obj, char* name, http_str_s def) {
    
    json_object* string = NULL;
    
    if (json_object_object_get_ex(obj, name, &string)) {
        
        http_str_s s;
        s.data   = (char*) json_object_get_string(string);
        s.length = strlen(s.data);
        
        return s;
        
    }
    
    return def;
    
}
