//
//  brook_kqueue.c
//  http-c-broker
//
//  Created by David Xavier on 28/07/2025.
//

#include "brook_kqueue.h"

int
brook_start_kernel_event (brook_config_t* conf) {
    
    // ... start event queue ...
    int kq = kqueue();
    struct kevent kq_list[MAX_EVENTS];
    
    // ... add event of server socket (accepts connections) and parent process to see if exit ...
    brook_kqueue_set_descriptor(kq, conf->socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    brook_kqueue_set_descriptor(kq, conf->brook_parent_process, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, NULL);
    
    // ... iterate event queue ...
    while (1) {
        int n = kevent(kq, NULL, 0, kq_list, MAX_EVENTS, NULL);
        
        for (int i = 0; i < n; i++) {
            brook_kevent_handle(kq, kq_list[i], conf);
        }
    }
    
    return BROOK_OK;
}

int
brook_kqueue_set_descriptor (int kq, int fd, int filter, int flags, int fflags, size_t data, void* udata) {
    struct kevent set;
    EV_SET(&set, fd, filter, flags, fflags, data, udata);
    kevent(kq, &set, 1, NULL, 0, NULL);
    return BROOK_OK;
}

void
brook_kevent_handle (int kq, struct kevent event, brook_config_t* conf) {
    
    switch (event.filter) {
        case EVFILT_READ:
            brook_evfilter_read(kq, event, conf);
            break;
        case EVFILT_TIMER:
            brook_close_connection(event.udata);
            break;
        case EVFILT_WRITE:
            break;
    }
}

// ------------------------------------------------- //
//                  Read Events Logic                //
// ------------------------------------------------- //

int
brook_evfilter_read (int kq, struct kevent event, brook_config_t* conf) {

    /*
     EVENTS TYPES:
        - new connections
        - read socket connection
        - read socket redis
        - read socket psql
     */
    
    
    // ... new connection in socket ...
    if (event.ident == conf->socket) {
        brook_connection_t* connection = brook_create_connection(conf);
        if (connection != NULL) {
            brook_kqueue_set_descriptor(kq, connection->socket, EVFILT_READ, EV_ADD, 0, 0, connection);
            brook_kqueue_set_descriptor(kq, connection->socket, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, conf->http.timeout, connection);
        }

    } else {
    // ... handle read events from state of connection
        if (event.udata == NULL) { return BROOK_DONE; }

        brook_connection_t* connection = event.udata;

        switch (connection->state) {
            case READING_SOCKET_MESSAGE: return brook_event_reading_socket_message(kq, event, connection); break;
            default: break;
        }

    }

    return BROOK_OK;
}

int
brook_event_reading_socket_message (int kq, struct kevent event, brook_connection_t* connection) {

    // ... read message of socket, is client socket ...
    int rs = brook_read_message_connection(connection);

    // ... if result is done need keep reading from socket ...
    if (rs == BROOK_DONE) return BROOK_DONE;

    // ... at this point we dont need read more from socket, because erro append or already has full message stored ...
    brook_kqueue_set_descriptor(kq, connection->socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);

    if (rs == BROOK_ERROR) {
        // ... in error only need create event to send response is, WRITE ...
        return BROOK_ERROR;
    }

    return BROOK_OK;
}
