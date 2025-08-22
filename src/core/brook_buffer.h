//
//  brook_buffer.h
//  http-c-broker
//
//  Created by David Xavier on 30/07/2025.
//

#ifndef brook_buffer_h
#define brook_buffer_h

#include "brook_config.h"

typedef struct brook_buffer_s brook_buffer_t;
typedef struct brook_chain_s  brook_chain_t;

/* STRUCT BUFFER
 * Store a buffer saving the start of buffer (init pointer) and the end of buffer (finish pointer)
 * The len is the amout data write in buffer
 * The size is the amout of memory alloced to buffer, max lenght basicly
 */
struct brook_buffer_s {
    u_char* start;
    u_char* end;
    size_t  length;
    size_t  size;
};

struct brook_chain_s {
    brook_buffer_t  buf;
    brook_chain_t*  next;
};

brook_buffer_t* brook_create_buffer(size_t size);

#endif /* brook_buffer_h */
