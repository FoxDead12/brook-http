//
//  brook_buffer.h
//  http-c-broker
//
//  Created by David Xavier on 30/07/2025.
//

#ifndef brook_buffer_h
#define brook_buffer_h

#include "brook_core.h"

typedef struct brook_buffer_s brook_buffer_t;
typedef struct brook_chain_s  brook_chain_t;

struct brook_buffer_s {
    u_char* start;
    u_char* end;
    size_t  len;
};

struct brook_chain_s {
    brook_buffer_t* buf;
    brook_chain_t*  next;
};

#endif /* brook_buffer_h */
