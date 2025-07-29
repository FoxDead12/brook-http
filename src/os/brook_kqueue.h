//
//  brook_kqueue.h
//  http-c-broker
//
//  Created by David Xavier on 28/07/2025.
//

#ifndef brook_kqueue_h
#define brook_kqueue_h

#include "brook_os.h"

int brook_start_kernel_event(brook_config_t* conf);
int brook_kqueue_set_descriptor(int kq, int fd, int filter, int flags, int fflags, void* udata);
void brook_kevent_handle(int kq, struct kevent event, brook_config_t* conf);
int brook_evfilter_read(int kq, struct kevent event, brook_config_t* conf);
#endif /* brook_kqueue_h */
