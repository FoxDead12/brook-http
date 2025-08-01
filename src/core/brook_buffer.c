//
//  brook_buffer.c
//  http-c-broker
//
//  Created by David Xavier on 01/08/2025.
//

#include "brook_buffer.h"

brook_buffer_t*
brook_create_buffer (size_t size) {
    
    brook_buffer_t* buff;
    buff->start = malloc(size);
    buff->end = buff->start + size;
    buff->size = size;
    buff->len = 0;
    
    return buff;
}
