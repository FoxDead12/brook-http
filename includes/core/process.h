#ifndef _BROOK_PROCESS_H
#define  _BROOK_PROCESS_H

#include "core/config.h"

int brook_process_start(brook_conf_t* config);
void brook_processes_shut_down_signals(int sig);
int brook_multi_processes_start(brook_conf_t* config, int num);
int brook_process_cleanup(brook_conf_t* config);

#endif
