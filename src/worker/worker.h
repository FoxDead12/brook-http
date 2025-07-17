//
//  worker.h
//  http-c-broker
//
//  Created by David Xavier on 25/06/2025.
//

#ifndef worker_h
#define worker_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../helpers/types/types.h"
#include "../socket/socket.h"
#include "../helpers/request/request.h"
#include "../db/db.h"
#include "../helpers/query/query.h"
#include "../helpers/response/response.h"

#ifdef __APPLE__
    #include "../os/mac/mac.h"
#endif

int     init_workers_processes (http_main_struct* conf);
pid_t   init_worker (http_main_struct* conf, int index);
int     worker_died (http_main_struct* conf, pid_t pid);
void    worker_event_loop (http_main_struct* conf);
int     worker_accept_new_connection (http_worker_struct* worker, http_connection_struct **con);
int     worker_close_connection (http_connection_struct* con);
int     worker_read_connection (http_connection_struct *con);
int     worker_build_and_send_async_query (http_connection_struct* con, int* socket);
int     worker_read_async_query (http_connection_struct* con, int socket);

#endif /* worker_h */
