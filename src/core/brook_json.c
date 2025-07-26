//
//  brook_json.c
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#include "brook_json.h"

json_object*
json_parse (char* data) {
    json_object* o = json_tokener_parse(data);
    free(data);
    return o;
}

int
json_get_int (const char* key, json_object* o, int def) {
    json_object* value = NULL;
    if (json_object_object_get_ex(o, key, &value)) {
        return json_object_get_int(value);
    }
    return def;
}

brook_array_t*
json_get_array (const char* key, json_object* o) {
    json_object* value = NULL;
    if (json_object_object_get_ex(o, key, &value)) {
        brook_array_t* a = brook_create_array(json_object_array_length(value));
        for (int i = 0; i < a->size; i++) {
            json_object* row = json_object_array_get_idx(value, i);
            brook_array_set_value(a, (u_char*) json_object_get_string(row), i);
        }
        return a;
    }
    return brook_create_array(0);
}
