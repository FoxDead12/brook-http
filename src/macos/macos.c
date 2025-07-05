//
//  macos.c
//  http-c-broker
//
//  Created by David Xavier on 27/06/2025.
//

#include "macos.h"
/*
int init_kqueue_loop (http_main_struct* conf) {
    
    int kq = kqueue();
    
    // add server socket/descripter to kernel kqueue
    add_descripter_to_queue(kq, conf->socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, NULL);
    
    // add main pid to kernel kqueue
    add_descripter_to_queue(kq, conf->pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, NULL);
    
    // set range of events to listening each iteraction of while
    struct kevent kq_event_list[MAX_EVENTS];
        
    while (1) {
        
        int num_events = kevent(kq, NULL, 0, kq_event_list, MAX_EVENTS, NULL);
                
        for (int i = 0; i < num_events; i++) {

            struct kevent event = kq_event_list[i];

            if (event.ident == conf->socket) { // new connection in server socket
                
                http_request_struct* client = socket_new_connection(conf);
                
                if (client != NULL) {
                    add_descripter_to_queue(kq, client->socket, EVFILT_READ, EV_ADD, 0, (void*) client);
                }

                
            } else if (event.flags & EV_EOF) { // disconect client socket
                
                http_request_struct *client = event.udata;
                
                add_descripter_to_queue(kq, client->socket, EVFILT_READ, EV_DELETE, 0, NULL);
                
                socket_disconect_connection(client);
                
            } else if (event.filter == EVFILT_READ) { // data receive from socket

                http_request_struct *client = event.udata;

                if (socket_new_message(client) == 1) {
                    //close connection because is finish the connection
                    add_descripter_to_queue(kq, client->socket, EVFILT_READ, EV_DELETE, 0, NULL);
                }
                
            } else if (event.fflags & NOTE_EXIT) { // parent process as dead
                exit(0);
            }
            
        }
        
    }
    
    return 0;
}

int add_descripter_to_queue (int kqueue, int fd, int filtro, int flags, int fflags, void* udata) {
    
    struct kevent set_ev;
    
    EV_SET(&set_ev, fd, filtro, flags, fflags, 0, udata);
    
    kevent(kqueue, &set_ev, 1, NULL, 0, NULL);
    
    return 0;
    
}
*/
