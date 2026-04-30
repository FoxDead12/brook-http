#include "core/config.h"
#include "core/process.h"
#include "core/configure.h"

// ... global shutdown state ...
brook_shutdown_t g_shutdown = {0, 0, 0};

brook_shutdown_t*
brook_shutdown_get(void) {
  return &g_shutdown;
}

void
brook_shutdown_init(void) {
  memset(&g_shutdown, 0, sizeof(g_shutdown));
}

void
brook_shutdown_cleanup(brook_conf_t* config) {
  // ... cleanup gatekeeper tree (config->root) ...
  if (config && config->root) {
    brook_gatekeeper_free_tree(config->root);
    config->root = NULL;
  }
  
  // ... close socket ...
  if (config && config->socket >= 0) {
    close(config->socket);
    config->socket = -1;
  }
  
  // ... close log file ...
  if (LOGGER_FILE != NULL && LOGGER_FILE != stdout && LOGGER_FILE != stderr) {
    fclose(LOGGER_FILE);
    LOGGER_FILE = NULL;
  }
}

// ... signal handler for graceful shutdown ...
static void
handle_signal(int sig) {
  if (sig == SIGTERM || sig == SIGINT) {
    g_shutdown.shutdown_requested = 1;
  } else if (sig == SIGHUP) {
    g_shutdown.reload_requested = 1;
  }
}

/**
 * Validate configuration at startup
 * Returns 0 on success, -1 on validation error
 */
static int
brook_validate_config (brook_conf_t* config) {
  if (!config) {
    fprintf(stderr, "Error: Configuration is NULL\n");
    return -1;
  }

  // ... validate port ...
  if (config->port <= 0 || config->port > 65535) {
    fprintf(stderr, "Error: Invalid port %d (must be 1-65535)\n", config->port);
    return -1;
  }

  // ... validate workers ...
  if (config->workers <= 0) {
    fprintf(stderr, "Error: Invalid workers %d (must be > 0)\n", config->workers);
    return -1;
  }

  // ... validate paths ...
  if (!config->gatekeeper[0]) {
    fprintf(stderr, "Error: Gatekeeper path not configured\n");
    return -1;
  }

  if (!config->log[0]) {
    fprintf(stderr, "Error: Log path not configured\n");
    return -1;
  }

  // ... validate pool settings (set defaults if not configured) ...
  if (config->pool.max_idle <= 0) config->pool.max_idle = 5;
  if (config->pool.min_idle < 0) config->pool.min_idle = 1;
  if (config->pool.max_lifetime <= 0) config->pool.max_lifetime = 3600;  // 1 hour
  if (config->pool.connect_timeout <= 0) config->pool.connect_timeout = 5000;  // 5s
  if (config->pool.read_timeout <= 0) config->pool.read_timeout = 30000;  // 30s
  if (config->pool.write_timeout <= 0) config->pool.write_timeout = 30000;  // 30s
  if (config->pool.retry_max <= 0) config->pool.retry_max = 3;
  if (config->pool.retry_delay <= 0) config->pool.retry_delay = 100;  // 100ms

  // ... validate beanstalkd config ...
  if (!config->beanstalkd.host[0] || config->beanstalkd.port <= 0) {
    fprintf(stderr, "Error: Beanstalkd not properly configured\n");
    return -1;
  }

  // ... validate redis config ...
  if (!config->redis.host[0] || config->redis.port <= 0) {
    fprintf(stderr, "Error: Redis not properly configured\n");
    return -1;
  }

  return 0;
}

int
main (int argc, char **argv) {

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

  // ... init shutdown state ...
  brook_shutdown_init();

  // ... setup signal handlers for graceful shutdown ...
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_signal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGHUP, &sa, NULL);

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
    // ... check for shutdown signal ...
    if (g_shutdown.shutdown_requested) {
      brook_log(config, LOG_INFO, "Shutdown signal received. Waiting for workers to finish...\n");
      break;
    }

    int status;
    pid_t dead_pid = waitpid(-1, &status, WNOHANG);

    if (dead_pid > 0) {
      if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        brook_log(config, LOG_WARN, "Process [%d] exited with code %d. Respawning...\n", dead_pid, exit_code);
      }
      else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        brook_log(config, LOG_WARN, "Process [%d] killed by signal %d (%s). Respawning...\n", dead_pid, sig, strsignal(sig));
      }
      // ... only respawn if shutdown not requested ...
      if (!g_shutdown.shutdown_requested) {
        brook_multi_processes_start(config, 1);
      }
    } else if (dead_pid == -1 && errno != ECHILD) {
      // ... no more children or error ...
      break;
    }
    
    // ... small sleep to prevent CPU spinning ...
    usleep(10000);  // 10ms
  }

  // ... graceful cleanup ...
  brook_log(config, LOG_INFO, "Cleaning up resources before exit...\n");
  brook_shutdown_cleanup(config);
  free(config);
  
  brook_log(config, LOG_INFO, "Server shutdown complete.\n");
  return BROOK_OK;
}
