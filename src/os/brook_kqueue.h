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
int brook_kqueue_set_descriptor(int kq, int fd, int filter, int flags, int fflags, size_t data, void* udata);
void brook_kevent_handle(int kq, struct kevent event, brook_config_t* conf);
int brook_kevent_read(brook_connection_t* connection, struct kevent event);
int brook_kevent_write(brook_connection_t* connection, struct kevent event);
int brook_kevent_user(int kq, brook_wait_list_t* list);

#endif /* brook_kqueue_h */
