#ifndef _BROOK_LOGGER_H
#define  _BROOK_LOGGER_H

#include "config.h"

#define RING_BUFFER_SIZE 1024

typedef enum {
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERR,

  LOG_LEVEL_LEN
} LOG_LEVEL;

extern char* type[LOG_LEVEL_LEN];

extern time_t logger_current_time;
extern struct tm * logger_time;

extern FILE* LOGGER_FILE;
extern int LOGGER_CURRENT_DAY;

void brook_log_init(brook_conf_t *config);
void brook_log(brook_conf_t* conf, LOG_LEVEL level, const char * fmt, ...);

#endif
