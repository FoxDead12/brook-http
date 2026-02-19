#include "core/config.h"
#include "core/process.h"

int
main(int argc, char **argv) {

  /***
   *  Hello,
   *   Here will be execute the HTTP server called "brook"
  */

  // ... create config of server ...
  brook_conf_t *config = malloc(sizeof(brook_conf_t));

  // ... init socket of server and add to config ...
  config->socket = brook_socket(3001);

  if ( config->socket == -1 ) {
    free(config);
    return BROOK_ERROR;
  }

  // ... start event loop ( for now is only one process ) ...
  brook_process_start(config);

  free(config);

  return BROOK_OK;
}
