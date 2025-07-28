//
//  brook_process.c
//  http-c-broker
//
//  Created by David Xavier on 27/07/2025.
//

#include "brook_process.h"


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
                printf("pid: %d\n", (pid_t*)conf->brook_processes->data[i]);
            }
        }
    }
    return BROOK_OK;
}

int
brook_start_single_process (brook_config_t* conf) {
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
    pid_t pid = fork();
    switch (pid) {
        case -1:
            perror("fork() failed\n");
            return BROOK_ERROR;
            break;
        case 0:
            sleep(1000);
            exit(1);
            break;
        default:
            break;
    }
    
    conf->brook_processes->data[i] = (void*) &pid;
    return BROOK_OK;
}

int
brook_died_process (brook_config_t* conf, pid_t pid) {
    for (int i = 0; i < conf->worker_processes; i++) {
        if (*(pid_t*)conf->brook_processes->data[i] == pid) {
            brook_array_set_value(conf->brook_processes, (void*) -1, i);
        }
    }
    return BROOK_OK;
}
