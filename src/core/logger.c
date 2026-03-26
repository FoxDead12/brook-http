#include "core/logger.h"
#include <errno.h>

time_t logger_current_time;
struct tm* logger_time;
FILE* logger_file = NULL;

void
brook_log_init () {
  logger_file = fopen("/Users/dxavier/Library/Logs/BrookHttp/brook-http.log", "a");
  // FILE* logger_dest = stdout;

  if (logger_file == NULL) {
    // Se der erro, o 'errno' dir-te-á porquê (ex: Permission Denied)
    printf("Erro ao abrir/criar: %s\n", strerror(errno));
  }
}

void
brook_log (LOG_LEVEL level, const char * fmt, ...) {
  va_list args;
  va_start(args, fmt);

  time(&logger_current_time);
  logger_time = localtime(&logger_current_time);

  fprintf(logger_file, "[%02d/%02d/%04dT%02d:%02d:%02d][%s][%s] ",
    logger_time->tm_mday,
    logger_time->tm_mon + 1, // tm_mon começa em 0 (Janeiro)
    logger_time->tm_year + 1900,
    logger_time->tm_hour,
    logger_time->tm_min,
    logger_time->tm_sec,
    brook_process_id,
    type[level]
  );

  vfprintf(logger_file, fmt, args);

  fflush(logger_file);

  va_end(args);
}
