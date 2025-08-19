//
//  brook_array.c
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#include "brook_array.h"
#include "brook_core.h"

brook_array_t*
brook_create_array (size_t size) {
    brook_array_t* a = malloc(sizeof(brook_array_t));
	a->data = malloc(sizeof(void*) * size);
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

int
brook_array_find_value (brook_array_t* a, brook_str_t s) {
    
    for (int i = 0;  i < a->size; i++) {
        int rs = brook_strncmp((char*) s.data, (char*) a->data[i], s.len);
        if (rs == 0) return i;
    }
    
    return -1;
}

