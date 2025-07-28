//
//  brook_process.h
//  http-c-broker
//
//  Created by David Xavier on 27/07/2025.
//

#ifndef brook_process_h
#define brook_process_h

#include "brook_core.h"

int brook_start_main_process(brook_config_t* conf);
int brook_start_single_process(brook_config_t* conf);
int brook_start_worker_process(brook_config_t* conf, int worker_processes);
int brook_spawn_process(brook_config_t* conf);
int brook_died_process(brook_config_t* conf, pid_t pid);
#endif /* brook_process_h */
