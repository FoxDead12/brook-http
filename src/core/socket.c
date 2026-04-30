#include "core/socket.h"

/**
 * Function to create socket server
 * will set all necessary configurations
 * and options to socket
 */
int
brook_socket (int PORT) {

  // ... ignore erros when write to invalid socket ...
  signal(SIGPIPE, SIG_IGN);

  // ... set address of server ...
  struct sockaddr_in _addr;
  _addr.sin_family = AF_INET;
  _addr.sin_port = htons(PORT);

  // ... olny allow localhost connections (only nginx will connect) ...
  _addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  // ... create socket descriptor ...
  int _s = socket(AF_INET, SOCK_STREAM, 0);
  if ( _s == -1 ) {
    perror("socket");
    exit(1);
  }

  // ... set socket non block ...
  if ( fcntl(_s, F_SETFL, fcntl(_s, F_GETFL, 0) | O_NONBLOCK) == -1 ) {
    perror("SO_REUSEPORT");
    exit(1);
  }

  // ... set option to multi process use same port ...
  int opt = 1;
  if ( setsockopt(_s, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int)) == -1 ) {
    perror("SO_REUSEPORT");
    exit(1);
  }

  // ... to enable server connect to same address ...
  if ( setsockopt(_s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1 ) {
    perror("SO_REUSEADDR");
    exit(1);
  }

  // ... make TCP await to fill a package to send (make a biger frame to send data) ...
  if ( setsockopt(_s, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1 ) {
    perror("TCP_NODELAY");
    exit(1);
  }

  // ... bind socket ...
  if ( bind(_s, (struct  sockaddr*) &_addr, sizeof(_addr)) == -1 ) {
    perror("bind");
    exit(1);
  }

  // ... set listen to socket ...
  if ( listen(_s, SOMAXCONN) == -1 ) {
    perror("listen");
    exit(1);
  }

  return _s;
}

/**
 * Function to close any socket
 */
int
brook_socket_close (int socket) {
  close(socket);
  return BROOK_OK;
}

/**
 * Function will handle new connections of socket
 */
int
brook_socket_accept (int socket, struct sockaddr* _addr, socklen_t* _addr_len) {

  // ... accept connection ...
  int _socket = accept(socket, _addr, _addr_len);

  // ... don't catch the connection is possible, multi process will try catch att same time ...
  if ( _socket <= 0 ) {
    return BROOK_DONE;
  }

  return _socket;
}

size_t brook_socket_recv ( int socket, unsigned char* b, size_t size ) {
  size_t _n = recv(socket, b, size, 0);
  return _n;
}
