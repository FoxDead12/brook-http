#include "core/logger.h"
#include <uuid/uuid.h>

FILE* LOGGER_FILE = NULL;
int LOGGER_CURRENT_DAY = 0;

time_t logger_current_time;
struct tm* logger_time;

void
brook_log_init ( brook_conf_t* config ) {
  // ... create path to logs files ...
  if (mkdir(config->log, 0755) == -1) {
    if (errno != EEXIST) {
      fprintf(stderr, "Error creating (%s) log directory: %s\n", config->log, strerror(errno));
      exit(1);
    }
  }

  // ... get current date ...
  char date_str[20];
  time_t t = time(NULL);
  struct tm* tm_info = localtime(&t);
  strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);

  // ... save date ...
  LOGGER_CURRENT_DAY = tm_info->tm_mday;

  // ... create final path ...
  char file_path[1024];
  snprintf(file_path, sizeof(file_path), "%s/brook-http-%s.log", config->log, date_str);

  if (LOGGER_FILE != NULL && LOGGER_FILE != stdout && LOGGER_FILE != stderr) {
    fclose(LOGGER_FILE);
    LOGGER_FILE = NULL;
  }

  // ... open file ...
  LOGGER_FILE = fopen(file_path, "a");

  if (LOGGER_FILE == NULL) {
    fprintf(stderr, "Failed to initialize log file at '%s': %s\n", file_path, strerror(errno));
    fprintf(stderr, "Warning: Logger falling back to standard output.\n");
    exit(1);
  }

  // ... init pool stats ...
  if (config) {
    memset(&config->pool_stats, 0, sizeof(config->pool_stats));
  }
}

void
brook_log (brook_conf_t* conf, LOG_LEVEL level, const char * fmt, ...) {

  time_t t = time(NULL);
  struct tm *tm_info = localtime(&t);

  if ( conf != NULL && tm_info->tm_mday != LOGGER_CURRENT_DAY ) {
    brook_log_init(conf);
  }

  va_list args;
  va_start(args, fmt);

  time(&logger_current_time);
  logger_time = localtime(&logger_current_time);

  fprintf(LOGGER_FILE, "[%02d-%02d-%04dT%02d:%02d:%02d.%06ld][%s][%s] ",
    tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900,
    tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
    (long)logger_current_time % 1000000,  // ... microseconds
    brook_process_id, type[level]
  );

  vfprintf(LOGGER_FILE, fmt, args);

  fflush(LOGGER_FILE);

  va_end(args);
}

/**
 * Generate a UUID v4 trace ID
 */
void
brook_trace_generate_id (char* buffer, size_t len) {
  uuid_t uuid;
  uuid_generate_random(uuid);
  uuid_unparse_lower(uuid, buffer);
  buffer[len - 1] = '\0';
}

/**
 * Initialize trace context for a request
 */
void
brook_trace_init (brook_trace_t* trace, const char* component) {
  if (!trace) return;
  
  brook_trace_generate_id(trace->trace_id, sizeof(trace->trace_id));
  
  // ... generate short span ID ...
  snprintf(trace->span_id, sizeof(trace->span_id), "%08x", 
    (unsigned int)(time(NULL) ^ getpid()));
  
  if (component) {
    strncpy(trace->component, component, sizeof(trace->component) - 1);
  } else {
    strncpy(trace->component, "unknown", sizeof(trace->component) - 1);
  }
  
  // ... use monotonic clock for duration tracking ...
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  trace->start_time = (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

/**
 * Get trace ID as string
 */
const char*
brook_trace_get_id (brook_trace_t* trace) {
  return trace ? trace->trace_id : "none";
}
