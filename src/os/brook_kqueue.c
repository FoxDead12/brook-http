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
    
    brook_kqueue_set_descriptor(kq, conf->socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    brook_kqueue_set_descriptor(kq, conf->brook_parent_process, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, NULL);
    
    while (1) {
        int n = kevent(kq, NULL, 0, kq_list, MAX_EVENTS, NULL);
        for (int i = 0; i < n; i++) {
            brook_kevent_handle(kq, kq_list[i], conf);
        }
    }
    return BROOK_OK;
}

void
brook_kevent_handle (int kq, struct kevent event, brook_config_t* conf) {
    if (event.filter == EVFILT_READ) {
        brook_evfilter_read(kq, event, conf);
    }
    
    if (event.filter == EVFILT_TIMER) {
        printf("tive um timoute\n");
    }
}


int
brook_kqueue_set_descriptor (int kq, int fd, int filter, int flags, int fflags, size_t data, void* udata) {
    struct kevent set;
    EV_SET(&set, fd, filter, flags, fflags, data, udata);
    kevent(kq, &set, 1, NULL, 0, NULL);
    return BROOK_OK;
}

// ------------------------------------------------- //
//                  Read Events Logic                //
// ------------------------------------------------- //

int
brook_evfilter_read (int kq, struct kevent event, brook_config_t* conf) {    
    if (event.ident == conf->socket) {
        
        brook_connection_t* connection = brook_create_connection(conf);
        
        if (connection != NULL) {
            brook_kqueue_set_descriptor(kq, connection->socket, EVFILT_READ, EV_ADD, 0, 0, connection);
            brook_kqueue_set_descriptor(kq, connection->socket, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, conf->http.timeout, connection);
        }
        
    } else {

        if (event.udata == NULL) { return BROOK_DONE; }

        brook_connection_t* connection = event.udata;

        switch (connection->state) {
            case READING_SOCKET_MESSAGE:
                return brook_event_reading_socket_message(kq, event, connection); break;
        }
        
    }
    
    return BROOK_OK;
}

int
brook_event_reading_socket_message (int kq, struct kevent event, brook_connection_t* connection) {
    
    int rs = brook_read_message_connection(connection);
    
    if (rs == BROOK_DONE) return BROOK_DONE;
    
    // if error and ok need remove event reader
    brook_kqueue_set_descriptor(kq, connection->socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    
    if (rs == BROOK_ERROR) {
        // need create event to write, when event was fire will send the response
        return BROOK_ERROR;
    }
    
    // catch all request
    // add event to execute query build
    
    
    
    return BROOK_OK;
}
