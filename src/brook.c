#include "core/config.h"
#include "core/process.h"
#include "core/configure.h"

int keep_running = 1; // ... variable to controll process, main and workers ...

/**
 * Brook main process singal shutdown handles
 */
void
brook_shut_down_signals ( int _ ) {
  keep_running = 0;
  return;
}

int
main (int argc, char **argv) {

  /***
   *  Hello,
   *   Here will be execute the HTTP server called "brook"
  */

  // ... set signals to controll shutdown ...
  signal(SIGINT, brook_shut_down_signals);
  signal(SIGTERM, brook_shut_down_signals);

  // ... check from user input ...
  if (argc < 2) {
    fprintf(stderr, "Error: Missing required argument [path]\n");
    fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
    return 1;
  }

  // ... create config of server ...
  brook_conf_t* config = malloc(sizeof(brook_conf_t));
  config->root = NULL;

  // ... load config file ...
  brook_load_configuration(config, argv[1]);

  // ... init logger ...
  brook_log_init(config);

  // ... init socket of server and add to config ...
  config->socket = brook_socket(config->port);
  if ( config->socket == -1 ) {
    free(config);
    return BROOK_ERROR;
  }

  // ... load gatekeeper file ...
  brook_gatekeeper_load(config);

#ifdef DEBUG_VSCODE
  brook_process_start(config);
  return BROOK_OK;
#endif

  // ... start event loop ( for now is only one process ) ...
  if ( brook_multi_processes_start(config, config->workers) == BROOK_ERROR ) {
    return BROOK_ERROR;
  }

  // ... manage workers ...
  while (keep_running) {
    int status;
    pid_t process_pid = waitpid(-1, &status, 0);

    switch (process_pid) {
      case -1: {
        brook_log(config, LOG_ERR, "waitpid error: %s\n", strerror(errno));
        break;
      }

      default: {

        // ... get returned code of process ...
        int exit_code = WEXITSTATUS(status);

        if ( keep_running == 0 ) {
          brook_log(config, LOG_WARN, "Process [%d] exited with code %d.\n", process_pid, exit_code);
          break;
        } else {
          brook_log(config, LOG_WARN, "Process [%d] exited with code %d. Respawning...\n", process_pid, exit_code);
        }

        brook_multi_processes_start(config, 1);
        break;
      }
    }
  }

  // ... kill all worker processes ...
  kill(0, SIGTERM);
  while (wait(NULL) > 0);

  brook_log(config, LOG_INFO, "All workers terminated. Cleaning up.\n");
  brook_log(config, LOG_INFO, "Main process shutdown.\n");

  free(config);
  return BROOK_OK;
}
