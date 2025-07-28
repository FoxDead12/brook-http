//
//  brook_array.h
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#ifndef brook_array_h
#define brook_array_h

#include "brook_core.h"

typedef struct brook_array_s brook_array_t;

struct brook_array_s {
    void** data;
    size_t   size;
};

brook_array_t* brook_create_array(size_t size, size_t type_size);
void brook_array_set_value(brook_array_t* array, void* data, int index);

#endif /* brook_array_h */
