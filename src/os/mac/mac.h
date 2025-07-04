//
//  mac.h
//  http-c-broker
//
//  Created by David Xavier on 04/07/2025.
//

#ifndef mac_h
#define mac_h

#include <stdio.h>
#include <sys/event.h> // import kqueue to mac os
#include "../../helpers/types/types.h"
#include "../../worker/worker.h"

#define MAX_KQ_EVENTS 64

int kqueue_init (http_worker_struct* worker);
int kqueue_set_descriptor (int kqueue, int fd, int filtro, int flags, int fflags, void* udata);
int handle_event (int kq, struct kevent e, http_worker_struct* worker);



#endif /* mac_h */
