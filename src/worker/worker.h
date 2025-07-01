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
#include "../helpers/types/types.h"
#include "../macos/macos.h"

int init_workers_processes (http_main_struct* conf);
int init_worker (http_main_struct* conf, int index);
int worker_died (http_main_struct* conf, pid_t pid);
int worker_event_loop (http_main_struct* conf);

#endif /* worker_h */
