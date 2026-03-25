#include "core/config.h"
#include "core/process.h"

int
main(int argc, char **argv) {

  /***
   *  Hello,
   *   Here will be execute the HTTP server called "brook"
  */

  brook_log(LOG_INFO, "Brook server start...\n");

  // ... create config of server ...
  brook_conf_t *config = malloc(sizeof(brook_conf_t));
  config->root = NULL;

  // ... init socket of server and add to config ...
  config->socket = brook_socket(6001);
  if ( config->socket == -1 ) {
    free(config);
    return BROOK_ERROR;
  }

  // ... load gatekeeper file ...
  brook_gatekeeper_load(config);

  // ... start event loop ( for now is only one process ) ...
  brook_process_start(config);

  free(config);

  return BROOK_OK;
}
