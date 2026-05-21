#include "core/process.h"
#include "core/beanstalkd.h"
#include "core/redis.h"

char                 brook_process_id[32] = {0};
int                  CURRENT_FD           = 0;
int                  MAX_FD               = 1024;   // ... max connections at same time ...
struct pollfd*       _fds                 = NULL;
brook_connection_t** _connections         = NULL;

/**
 * Method to create worker processes
 */
int
brook_multi_processes_start (brook_conf_t* config, int num) {
  pid_t pid;

  for (int i = 0; i < num; i++ ) {
    pid = fork();

    if (pid < 0) {
      perror("fork");
      return BROOK_ERROR;
    }

    if (pid == 0) {
      brook_process_start(config);
      sleep(2);
      exit(0);
      return BROOK_OK;
    }
  }
  return BROOK_OK;
}

/**
 * Brook worker process singal shutdown handles
 */
void
brook_processes_shut_down_signals ( int sing ) {
  (void)(sing);
  keep_running = 0;
  return;
}

/**
 * Process logic, will run event loop logic,
 * handle new connections and manager HTTP
 * connections
 */
int
brook_process_start ( brook_conf_t* config ) {

  // ... set signals to controll shutdown ...
  signal(SIGINT, brook_processes_shut_down_signals);
  signal(SIGTERM, brook_processes_shut_down_signals);

  // ... set values of global variables of process ...
  int static_fds = 4;         // ... for now is only tcp socket of server and beanstalkd client and two redis client

  // ... alloc memory to all files descriptors connections ...
  {
    _fds = malloc(sizeof(struct pollfd) * (MAX_FD + static_fds));

    if ( !_fds ) {
      brook_log(config, LOG_ERR, "Malloc failed at %s:%d: %s\n", __FILE__, __LINE__, strerror(errno));
      return BROOK_ERROR;
    }
  }

  // ... alloc memory to store all connections HTTP info ...
  {
    _connections = malloc(sizeof(brook_connection_t*) * (MAX_FD + static_fds));

    if ( !_connections ) {
      brook_log(config, LOG_ERR, "Malloc failed at %s:%d: %s\n", __FILE__, __LINE__, strerror(errno));
      return BROOK_ERROR;
    }
  }

  // ... generate id of process ...
  pid_t current_pid = getpid();
  snprintf(brook_process_id, sizeof(brook_process_id), "brook-%d", (int)current_pid);

  brook_log(config, LOG_INFO, "Process is setuping ...\n");

  // ... connect to beanstalkd ...
  if ( brook_beanstalkd_connect(config) == BROOK_ERROR ) {
    brook_log(config, LOG_ERR, "Can't create beanstalkd client: %s (errno: %d)\n", strerror(errno), errno);
    return BROOK_ERROR;
  }

  // ... connect to redis ...
  if ( brook_redis_connect(config, 0) == BROOK_ERROR ) {
    brook_log(config, LOG_ERR, "Can't create redis client: %s (errno: %d)\n", strerror(errno), errno);
    return BROOK_ERROR;
  }

  // ... connect to redis subescriber ...
  if ( brook_redis_connect(config, 1) == BROOK_ERROR ) {
    brook_log(config, LOG_ERR, "Can't create redis client: %s (errno: %d)\n", strerror(errno), errno);
    return BROOK_ERROR;
  }

  // ... clean struct ...
  for ( int i = 0; i < MAX_FD; i++ ) {
    _fds[i].fd = -1;
    _fds[i].events = 0;
    _fds[i].revents = 0;
  }

  // ... set in poll the server socket ...
  // ... will has two types of sockets in fd (socket server, beanstalkd client socket)
  _fds[0].fd = config->socket;
  _fds[0].events = POLLIN;
  _fds[0].revents = 0;

  _fds[POOL_INDEX_BEANSTALKD].fd = bean_client->fd;
  _fds[POOL_INDEX_BEANSTALKD].events = POLLIN | POLLOUT;
  _fds[POOL_INDEX_BEANSTALKD].revents = 0;

  _fds[POOL_INDEX_REDIS].fd = redis_client->c.fd;
  _fds[POOL_INDEX_REDIS].events = POLLIN | POLLOUT;
  _fds[POOL_INDEX_REDIS].revents = 0;

  _fds[POOL_INDEX_REDIS_SUBSCRIBER].fd = redis_client_sub->c.fd;
  _fds[POOL_INDEX_REDIS_SUBSCRIBER].events = POLLIN | POLLOUT;
  _fds[POOL_INDEX_REDIS_SUBSCRIBER].revents = 0;

  brook_log(config, LOG_INFO, "Process start to work ...\n");

  // ... event loop start here ...
  while (keep_running == 1) {

    // ... set num of current file descriptors ...
    int t = CURRENT_FD + static_fds;

    // ... wait for events in sockets/file descriptors ...
    int nready = poll(_fds, t, -1);

    if ( nready == -1 ) {
      keep_running = 0;
      break;
    }

    // ... check all descriptors ...
    for ( int i = 0; i < t; i++ ) {

      struct pollfd* _fd = &_fds[i];

      // ... ignore empty index's ...
      if ( _fd->fd == -1 ) {
        continue;
      }

      if ( _fd->fd == config->socket && _fd->revents & POLLIN ) {
        // ... need accept TCP connection ...
        brook_handle_connection(config);

      } else if ( _fd->fd == bean_client->fd ) {
        // ... beanstalkd data to read ...
        if ( _fd->revents & POLLIN ) {
          bsc_read(bean_client);
        }
        // ... beanstalkd data to write
        if ( _fd->revents & POLLOUT ) {
          brook_benstalkd_write();
        }

      } else if ( _fd->fd == redis_client->c.fd ) {
        // ... redis data to read ...
        if ( _fd->revents & POLLIN ) {
          redisAsyncHandleRead(redis_client);
        }
        // ... redis data to write
        if ( _fd->revents & POLLOUT ) {
          brook_redis_write(redis_client, POOL_INDEX_REDIS);
        }

      } else if ( _fd->fd == redis_client_sub->c.fd ) {
        // ... redis data to read ...
        if ( _fd->revents & POLLIN ) {
          redisAsyncHandleRead(redis_client_sub);
        }
        // ... redis data to write
        if ( _fd->revents & POLLOUT ) {
          brook_redis_write(redis_client_sub, POOL_INDEX_REDIS_SUBSCRIBER);
        }

      }
      else {
        if ( _fd->revents & (POLLHUP | POLLERR ) ) {
          brook_connection_t* con = _connections[i];
          if ( con != NULL ) {
            brook_destroy_connection(con);
          }

        } else if ( _fd->revents & POLLIN ) {
          // ... events de leitura dos sockets ...
          brook_connection_t* con = _connections[i];
          if ( con != NULL ) {
            brook_connection_read(con);
          }

        } else if ( _fd->revents & POLLOUT ) {
          brook_connection_t* con = _connections[i];
          if (con != NULL) {
            brook_connection_write(con);
          }

        }
      }
    }
  }

  // ... clean memory ...
  brook_process_cleanup(config);

  return BROOK_OK;
}

/**
 * Method to close process, worker process
 */
int
brook_process_cleanup ( brook_conf_t* config ) {

  brook_log(config, LOG_INFO, "Cleaning up process %s before exit...\n", brook_process_id);

  // ... close all connections ...
  for (int i = 0; i < MAX_FD; i++) {
    if (_connections && _connections[i] != NULL) {
      brook_destroy_connection(_connections[i]);
    }
  }

  if (_fds) {
    free(_fds);
    _fds = NULL;
  }

  if (_connections) {
    free(_connections);
    _connections = NULL;
  }

  if (bean_client != NULL) {
    if (bean_client->fd >= 0) {
      close(bean_client->fd);
    }
    free(bean_client);
    bean_client = NULL;
  }

  close(redis_client->c.fd);
  close(redis_client_sub->c.fd);

  brook_log(config, LOG_INFO, "Process %s terminated safely.\n", brook_process_id);

  // ... block for processor controll ...
  return BROOK_OK;
}
