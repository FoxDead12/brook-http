#include "core/beanstalkd.h"

int
brook_beanstalkd_connect () {

  char errorstr[1024];
  bsc *client = bsc_new("localhost", "11301", "", brook_beanstalkd_on_error, 4096, 16, 4, errorstr);

  if ( client == NULL ) {
    fprintf(stderr, "Client Beanstalkd can't connect: %s\n", errorstr);
    return 1;
  }

  bsc_error_t bsc_error = bsc_put(client, NULL, NULL, 1, 0, 10, strlen("baba"), "baba", false);

  return BROOK_OK;
}

void
brook_beanstalkd_on_error ( struct _bsc *b, bsc_error_t error_code ) {
  const char *err_msg = "UNKNOWN";
  // Mapeamento baseado no enum que enviaste
  switch (error_code) {
    case BSC_ERROR_NONE:         err_msg = "None (Success)"; break;
    case BSC_ERROR_INTERNAL:     err_msg = "Internal Error"; break;
    case BSC_ERROR_SOCKET:       err_msg = "Socket Error (Connection/IO)"; break;
    case BSC_ERROR_MEMORY:       err_msg = "Memory Allocation Error"; break;
    case BSC_ERROR_QUEUE_FULL:   err_msg = "Command Queue Full"; break;
    default:                     err_msg = "Undocumented Error"; break;
  }
  fprintf(stderr, "[Beanstalk Event] Error Code: %d (%s)\n", error_code, err_msg);
}
