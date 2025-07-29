//
//  brook_kqueue.c
//  http-c-broker
//
//  Created by David Xavier on 28/07/2025.
//

#include "brook_kqueue.h"

int
brook_start_kernel_event (brook_config_t* conf) {
    
    int kq = kqueue();
    struct kevent kq_list[MAX_EVENTS];
    
    brook_kqueue_set_descriptor(kq, conf->socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, NULL);
    brook_kqueue_set_descriptor(kq, conf->brook_parent_process, EVFILT_PROC, EV_ADD, NOTE_EXIT, NULL);\
    while (1) {
        int n = kevent(kq, NULL, 0, kq_list, MAX_EVENTS, NULL);
        for (int i = 0; i < n; i++) {
            brook_kevent_handle(kq, kq_list[i], conf);
        }
    }
    return BROOK_OK;
}

void brook_kevent_handle (int kq, struct kevent event, brook_config_t* conf) {
    
    if (event.filter == EVFILT_READ) {
        brook_evfilter_read(event, conf);
    }
    
}

int
brook_kqueue_set_descriptor (int kq, int fd, int filter, int flags, int fflags, void* udata) {
    struct kevent set;
    EV_SET(&set, fd, filter, flags, fflags, 0, udata);
    kevent(kq, &set, 1, NULL, 0, NULL);
    return BROOK_OK;
}

int
brook_evfilter_read (struct kevent event, brook_config_t* conf) {
    
    if (event.ident == conf->socket) {
        brook_create_connection(conf);
    }
    
    return BROOK_OK;
}
