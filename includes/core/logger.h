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

// typedef struct {
//   brook_str_t message;
// } log_node_s;
//
// typedef struct {
//   log_node_s queue[RING_BUFFER_SIZE];
//   atomic_size_t write;
//   size_t read;
// } log_ring;

static char * type[LOG_LEVEL_LEN] = {
  "DEBUG",
  "INFO",
  "WARN",
  "ERRO"
};


extern time_t logger_current_time;
extern struct tm * logger_time;
extern FILE* logger_file;

void brook_log_init();
void brook_log (LOG_LEVEL level, const char * fmt, ...);

#endif
