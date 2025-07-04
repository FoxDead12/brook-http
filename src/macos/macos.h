//
//  macos.h
//  http-c-broker
//
//  Created by David Xavier on 27/06/2025.
//

#ifndef macos_h
#define macos_h

#include <stdio.h>
#include <sys/event.h> // import kqueue to mac os
#include "../socket/socket.h"
#include "../helpers/types/types.h"

#define MAX_EVENTS 24

/*
int init_kqueue_loop (http_main_struct* conf);
int add_descripter_to_queue (int kqueue, int fd, int filtro, int flags, int fflags, void* udata);
*/
#endif /* macos_h */
