#include "core/process.h"
#include "core/beanstalkd.h"
#include "core/redis.h"

int MAX_FD = 1024;   // ... max connections at same time ...
int CURRENT_FD = 0;
char brook_process_id[32];

brook_connection_t** _connections = NULL;
struct pollfd* _fds = NULL;

int
brook_multi_processes_start (brook_conf_t* config, int num) {
  pid_t pid;

  for (int i = 0; i < num; i++ ) {
    pid = fork();

    if (pid < 0) {
      perror("Erro ao criar processo filho");
    }
    if (pid == 0) {
      brook_process_start(config);
      exit(0);
    }
  }
  return BROOK_OK;
}


/**
 * Process logic, will run event loop logic,
 * handle new connections and manager HTTP
 * connections
 */
int
brook_process_start ( brook_conf_t* config ) {

  #ifdef __linux__
    prctl(PR_SET_PDEATHSIG, SIGTERM);
  #endif

  // ... set values of global variables of process ...
  int static_fds = 3;         // ... for now is only tcp socket of server and beanstalkd client and redis client

  _fds = malloc(sizeof(struct pollfd) * (MAX_FD + static_fds));
  _connections = malloc(sizeof(brook_connection_t*) * (MAX_FD + static_fds));

  // ... generate id of process ...
  pid_t current_pid = getpid();
  snprintf(brook_process_id, sizeof(brook_process_id), "brook-%d", (int)current_pid);

  brook_log(config, LOG_INFO, " Process is starting ...\n");

  if ( brook_beanstalkd_connect(config) == BROOK_ERROR ) {
    char *err_desc = strerror(errno);
    brook_log(config, LOG_ERR, "Can't create beanstalkd client: %s (errno: %d)\n", err_desc, errno);
    perror("brook_beanstalkd_connect");
    return BROOK_ERROR;
  }

  if ( brook_redis_connect(config) == BROOK_ERROR ) {
    char *err_desc = strerror(errno);
    brook_log(config, LOG_ERR, "Can't create redis client: %s (errno: %d)\n", err_desc, errno);
    perror("brook_redis_connect");
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

  CURRENT_FD += static_fds;

  // ... event loop start here ...
  while (1) {

  #ifndef __linux__
    if (getppid() == 1) {
      brook_log(config, LOG_ERR, "Orphan child detected. Shutting down.\n");
      break;
    }
    #endif

    // ... wait for events in sockets/file descriptors ...
    int nready = poll(_fds, CURRENT_FD, -1);
    if ( nready == -1 ) {
      perror("poll");
      return BROOK_ERROR;
    }

    int t = CURRENT_FD; // ... this dont make sense only for first iteration when server start clean

    // ... check all descriptors ...
    for ( int i = 0; i < t; i++ ) {
      struct pollfd* _fd = &_fds[i];

      // ... ignore empty index's ...
      if ( _fd->fd == -1 ) continue;

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
          brook_redis_write();
        }

      } else {
        if ( _fd->revents & POLLIN ) {
          // ... events de leitura dos sockets ...
          brook_connection_t* con = _connections[i];
          brook_connection_read(con);

        } else if ( _fd->revents & POLLOUT ) {
          brook_connection_t* con = _connections[i];
          brook_connection_write(con);

        } else if ( _fd->revents & (POLLHUP | POLLERR) ) {
          brook_connection_t* con = _connections[i];
          if ( con != NULL ) {
            brook_destroy_connection(con);
          }

        }
      }
    }
  }

  return BROOK_OK;
}
