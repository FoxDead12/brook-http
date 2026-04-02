#include "core/config.h"
#include "core/process.h"
#include "core/configure.h"

int
main(int argc, char **argv) {

  /***
   *  Hello,
   *   Here will be execute the HTTP server called "brook"
  */

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
  brook_multi_processes_start(config, config->workers);

  while (1) {
    int status;
    pid_t dead_pid = wait(&status);

    if (dead_pid > 0) {
      if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        brook_log(config, LOG_WARN, "Process [%d] exited with code %d. Respawning...\n", dead_pid, exit_code);
      }
      else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        brook_log(config, LOG_WARN, "Process [%d] killed by signal %d (%s). Respawning...\n", dead_pid, sig, strsignal(sig));
      }
      brook_multi_processes_start(config, 1);
    } else if (dead_pid == -1 && errno != EINTR) {
      break;
    }
  }

  free(config);
  return BROOK_OK;
}
