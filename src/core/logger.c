#include "core/logger.h"

time_t logger_current_time;
struct tm * logger_time;

void
brook_log_init () {

}

void
brook_log (LOG_LEVEL level, const char * fmt, ...) {
  va_list args;
  va_start(args, fmt);

  time(&logger_current_time);
  logger_time = localtime(&logger_current_time);

  printf("[%d/%d/%dT%d:%d:%d][%s] ",
    logger_time -> tm_mday,
    logger_time -> tm_mon,
    logger_time -> tm_year + 1900,
    logger_time -> tm_hour,
    logger_time -> tm_min,
    logger_time -> tm_sec,
    type[level]
  );

  vfprintf(stdout, fmt, args);
  va_end(args);
}
