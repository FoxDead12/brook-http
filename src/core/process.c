#include "core/process.h"
#include "core/beanstalkd.h"

int MAX_FD = 1024;   // ... max connections at same time ...
int CURRENT_FD = 0;

/**
 * Process logic, will run event loop logic,
 * handle new connections and manager HTTP
 * connections
 */
int
brook_process_start ( brook_conf_t* config ) {

  printf("[%d] Process will start event loop\n", getpid());

  // ... set values of global variables of process ...
  _fds = malloc(sizeof(struct pollfd) * MAX_FD);
  _connections = malloc(sizeof(brook_connection_t*) * MAX_FD);

  if ( brook_beanstalkd_connect() == BROOK_ERROR ) {
    perror("brook_beanstalkd_connect");
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
  _fds[0].revents = POLLIN;

  _fds[1].fd = bean_client->fd;
  _fds[1].events = POLLIN;
  _fds[1].revents = POLLIN;

  int static_fds = 2;         // ... for now is only tcp socket of server and beanstalkd client
  // ... event loop start here ...
  while (1) {

    // ... wait for events in sockets/file descriptors ...
    int nready = poll(_fds, CURRENT_FD + static_fds, -1);
    if ( nready == -1 ) {
      perror("poll");
      return BROOK_ERROR;
    }

    int t = CURRENT_FD + static_fds;
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
