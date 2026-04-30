#ifndef _BROOK_LOGGER_H
#define  _BROOK_LOGGER_H

#include "config.h"

#define RING_BUFFER_SIZE 1024

typedef enum {
  LOG_TRACE,    // ... detailed execution flow
  LOG_DEBUG,    // ... debug info
  LOG_INFO,     // ... general info
  LOG_WARN,     // ... warnings
  LOG_ERROR,    // ... recoverable errors
  LOG_FATAL,    // ... critical errors
  LOG_CRITICAL, // ... system failure

  LOG_LEVEL_LEN
} LOG_LEVEL;

static char * type[LOG_LEVEL_LEN] = {
  "TRACE",
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL",
  "CRITICAL"
};

extern time_t logger_current_time;
extern struct tm * logger_time;

extern FILE* LOGGER_FILE;
extern int LOGGER_CURRENT_DAY;

void brook_log_init(brook_conf_t *config);
void brook_log(brook_conf_t* conf, LOG_LEVEL level, const char * fmt, ...);

// ... trace functions for observability ...
void brook_trace_init(brook_trace_t* trace, const char* component);
void brook_trace_generate_id(char* buffer, size_t len);
const char* brook_trace_get_id(brook_trace_t* trace);

// ... macro for trace-aware logging ...
#define brook_log_trace(conf, level, trace_ctx, fmt, ...) \
  brook_log(conf, level, "[trace=%s] " fmt, \
    trace_ctx ? brook_trace_get_id(trace_ctx) : "none", ##__VA_ARGS__)

#endif
