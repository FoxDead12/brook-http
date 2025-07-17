//
//  mac.c
//  http-c-broker
//
//  Created by David Xavier on 04/07/2025.
//

#include "mac.h"

int kqueue_init (http_worker_struct* worker) {
    
    int kq = kqueue();
    struct kevent kq_event_list[MAX_KQ_EVENTS];
    
    kqueue_set_descriptor(kq, worker->server->socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, NULL);
    kqueue_set_descriptor(kq, worker->server->pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, NULL);
            
    while (1) {
        
        int n = kevent(kq, NULL, 0, kq_event_list, MAX_KQ_EVENTS, NULL);
                
        for (int i = 0; i < n; i++) {
            handle_event(kq, kq_event_list[i], worker);
        }
        
    }
    
    return HTTP_OK;
}

int kqueue_set_descriptor (int kqueue, int fd, int filtro, int flags, int fflags, void* udata) {
    struct kevent set_ev;
    EV_SET(&set_ev, fd, filtro, flags, fflags, 0, udata);
    kevent(kqueue, &set_ev, 1, NULL, 0, NULL);
    return HTTP_OK;
}

int handle_event (int kq, struct kevent e, http_worker_struct* worker) {
    
    if (e.ident == worker->server->socket) {
        
        // server socket accept new connection
        http_connection_struct* conn = NULL;
        
        if (worker_accept_new_connection(worker, &conn) == HTTP_OK) {
            kqueue_set_descriptor(kq, conn->socket, EVFILT_READ, EV_ADD, 0, (void*) conn);
        }
        
    } else if (e.filter == EVFILT_READ) {
        
        http_connection_struct* con = e.udata;
        
        if ( con->status == 0 ) {
            
            int r = worker_read_connection(con);
            
            if (r != HTTP_DONE) {
                
                kqueue_set_descriptor(kq, con->socket, EVFILT_READ, EV_DELETE, 0, NULL);
                
                if (r == HTTP_OK) {
                    // add user trigger
                    kqueue_set_descriptor(kq, con->socket, EVFILT_USER, EV_ADD, NOTE_FFCOPY, NULL);
                    kqueue_set_descriptor(kq, con->socket, EVFILT_USER, EV_ENABLE, NOTE_TRIGGER, (void*) con);
                    
                } else if (r == HTTP_ERROR) {
                    worker_close_connection(con);
                }
                
            }
            
        } else if (con->status == 2) {

            http_connection_struct* con = e.udata;
            
            if (worker_read_async_query(con, (int) e.ident) != HTTP_DONE) {
                kqueue_set_descriptor(kq, (int) e.ident, EVFILT_READ, EV_DELETE, 0, NULL);
            }

        }
        
                    
    } else if (e.filter == EVFILT_USER) {
        
        // my custom event, is to build query
        http_connection_struct* con = e.udata;
        
        const char* query = "SELECT * FROM users u LEFT JOIN products p ON u.id = p.user_id";
        
        int db_socket;
        int r = worker_build_and_send_async_query(con, (char*) query, &db_socket);

        if (r != HTTP_DONE) {
            kqueue_set_descriptor(kq, con->socket, EVFILT_USER, EV_DELETE, 0, NULL);
            kqueue_set_descriptor(kq, db_socket, EVFILT_READ, EV_ADD, 0, (void*) con);
        }
        
    } else if (e.flags & EV_EOF)  {
        
        http_connection_struct* con = e.udata;
        worker_close_connection(con);
        
    } else if (e.fflags & NOTE_EXIT) {
        
        exit(HTTP_OK);
        
    }
    
    return HTTP_OK;
}
