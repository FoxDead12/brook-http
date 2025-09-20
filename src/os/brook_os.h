//
//  brook_os.h
//  http-c-broker
//
//  Created by David Xavier on 28/07/2025.
//

#ifndef brook_os_h
#define brook_os_h

#include "../core/brook_core.h"

typedef struct brook_wait_list_s brook_wait_list_t;

struct brook_wait_list_s {
	brook_connection_t* c;
    brook_wait_list_t* next;
};


#ifdef __APPLE__
    #include <sys/event.h>
    #include "brook_kqueue.h"
#endif

#define MAX_EVENTS 1024

#endif /* brook_os_h */
