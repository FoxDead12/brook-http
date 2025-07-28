//
//  brook_array.c
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#include "brook_array.h"

brook_array_t*
brook_create_array (size_t size, size_t type_size) {
    brook_array_t* a = malloc(sizeof(brook_array_t));
    a->data = malloc(type_size * size);
    a->size = size;
    return a;
}

void
brook_array_clear (brook_array_t* a, bool free_values) {
    for (int i = 0; i < a->size; i++) {
        free(a->data[i]);
    }
    free(a->data);
    free(a);
}

void
brook_array_set_value (brook_array_t* array, void* data, int index) {
    array->data[index] = data;
}
