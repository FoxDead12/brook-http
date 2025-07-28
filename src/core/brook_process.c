//
//  brook_process.c
//  http-c-broker
//
//  Created by David Xavier on 27/07/2025.
//

#include "brook_process.h"
#include "../os/brook_os.h"

int
brook_start_main_process (brook_config_t* conf) {
    brook_start_worker_process(conf, conf->worker_processes);
    while(1) {
        int status;
        pid_t child = wait(&status);
        if (child >= 0) {
            brook_died_process(conf, child);
            brook_spawn_process(conf);
            for (int i = 0; i < conf->worker_processes; i++) {
                printf("pid: %d\n", *(int*)conf->brook_processes->data[i]);
            }
        }
    }
    return BROOK_OK;
}

int
brook_start_single_process (brook_config_t* conf) {
    brook_start_event_loop_process(conf);
    return BROOK_OK;
}

int
brook_start_worker_process (brook_config_t* conf, int worker_processes) {
    for (int i = 0; i < worker_processes; i++) {
        brook_spawn_process(conf);
    }
    return BROOK_OK;
}

int
brook_spawn_process (brook_config_t* conf) {
    int i;
    for (i = 0; i < conf->worker_processes; i++) {
        if (conf->brook_processes->data[i] == (void*) -1) {
            break;
        }
    }
    pid_t p = fork();
    switch (p) {
        case -1:
            perror("fork() failed\n");
            return BROOK_ERROR;
            break;
        case 0:
            brook_start_event_loop_process(conf);
            break;
        default:
            break;
    }
    pid_t* pid = malloc(sizeof(pid_t));
    *pid = p;
    brook_array_set_value(conf->brook_processes, pid, i);
    return BROOK_OK;
}

int
brook_died_process (brook_config_t* conf, pid_t pid) {
    for (int i = 0; i < conf->worker_processes; i++) {
        if (*(pid_t*)conf->brook_processes->data[i] == pid) {
            free(conf->brook_processes->data[i]);
            brook_array_set_value(conf->brook_processes, (void*) -1, i);
        }
    }
    return BROOK_OK;
}

void
brook_start_event_loop_process (brook_config_t* conf) {
    // store config of process and init examples database connections
    conf->brook_process.pid = getpid();
    
    brook_start_kernel_event(conf);
    exit(1);
}
