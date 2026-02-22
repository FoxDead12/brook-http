#include "core/process.h"

int MAX_CLIENTS = 1024;   // ... max connections at same time ...

/**
 * Process logic, will run event loop logic,
 * handle new connections and manager HTTP
 * connections
 */
int
brook_process_start ( brook_conf_t* config ) {

  printf("[%d] Process will start event loop\n", getpid());

  // ... set values of global variables of process ...
  _fds = malloc(sizeof(struct pollfd) * MAX_CLIENTS);
  _connections = malloc(sizeof(brook_connection_t*) * MAX_CLIENTS);

  // ... clean struct ...
  for ( int i = 0; i < MAX_CLIENTS; i++ ) {
    _fds[i].fd = -1;
    _fds[i].events = POLLIN;
  }

  // ... set in poll the server socket ...
  // ... will has two types of sockets in fd (socket server, beanstalkd client socket)
  _fds[0].fd = config->socket;

  // ... event loop start here ...
  while (1) {

    // ... wait for events in sockets/file descriptors ...
    int nready = poll(_fds, MAX_CLIENTS, -1);
    if ( nready == -1 ) {
      perror("poll");
      return BROOK_ERROR;
    }

    // ... check all descriptors ...
    for ( int i = 0; i < MAX_CLIENTS; i++ ) {
      struct pollfd* _fd = &_fds[i];

      // ... ignore empty index's ...
      if ( _fd->fd == -1 ) continue;

      if ( _fd->fd == config->socket && _fd->revents & POLLIN ) {
        // ... need accept TCP connection ...
        brook_handle_connection(config);

      } else if ( _fd->revents & POLLIN ) {
        // ... events de leitura dos sockets ...
        brook_connection_t* con = _connections[i];
        int r = brook_connection_read(con);
        if ( r == BROOK_ERROR ) {
          // ... TODO: handle erros of connection read, need destroy objects and responde to client
          // TODO: generate response error ...
          brook_destroy_connection(con);
        }
      }


    }

  }

  return BROOK_OK;
}
