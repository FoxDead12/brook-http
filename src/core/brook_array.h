//
//  brook_array.h
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#ifndef brook_array_h
#define brook_array_h

#include "brook_config.h"

typedef struct brook_array_s brook_array_t;
typedef struct brook_str_s   brook_str_t;


struct brook_array_s {
    void** data;
    size_t size;
};

brook_array_t* brook_create_array(size_t size);
void brook_array_clear(brook_array_t* a, bool free_values);
void brook_array_set_value(brook_array_t* array, void* data, int index);
int brook_array_find_value(brook_array_t* a, brook_str_t s);

#endif /* brook_array_h */
