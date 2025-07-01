//
//  worker.c
//  http-c-broker
//
//  Created by David Xavier on 25/06/2025.
//

#include "worker.h"

int init_workers_processes (http_main_struct* conf) {
    
    // create children process to handle connections
    for (int i = 0; i < conf->worker_processes; i++) {
        init_worker(conf, i);
    }
    
    return 0;
}

int init_worker (http_main_struct* conf, int index) {
    
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork() can't create process");
        return -1;
    }
    
    if (pid == 0) {
        worker_event_loop(conf);
        exit(0);
    }
    
    if (pid > 0) {
        setpgid(pid, getpgrp());
        conf->workers[index].pid = pid; // update parent process to store children pid
    }
    
    return 0;
}

int worker_died (http_main_struct* conf, pid_t pid) {
    
    printf("worker process died %d\n", pid);

    for (int i = 0; i < conf->worker_processes; i++) {
        if (conf->workers[i].pid == pid) {
            init_worker(conf, i);
        }
    }
    
    return 0;
}

int worker_event_loop (http_main_struct* conf) {

    // where is necessary to handle kqueue in mac os and epoll in linux
    printf("worker process start %d\n", getpid());

    init_kqueue_loop(conf);
    
    return 0;
}
