#include "core/connection.h"
#include "http/brook_http_request.h"

int
brook_handle_connection ( brook_conf_t* config ) {
  // ... accept file descriptor connection ...
  struct sockaddr_in _addr;
  socklen_t addr_len = sizeof(_addr);

  // ... accept connection ...
  int _socket = brook_socket_accept(config->socket, (struct sockaddr*) &_addr, &addr_len);
  if ( _socket == BROOK_DONE ) {
    return BROOK_DONE;
  }

  printf("Nova conexao estabelecida\n");

  // ... create connection struct and
  brook_connection_t* con = malloc(sizeof(brook_connection_t));
  con->_config = config;
  con->_fd = _socket;
  con->_status = 0;
  con->_data = NULL;
  con->_port = ntohs(_addr.sin_port);
  inet_ntop(AF_INET, &(_addr.sin_addr), con->_ip, INET_ADDRSTRLEN);

  con->_parser = malloc(sizeof(brook_http_parse_t));
  con->_parser->content_length = 0;
  con->_parser->http_minor = 0;
  con->_parser->method = 0;
  con->_parser->nread = 0;
  con->_parser->state = 0;

  // ... add conection to list ...
  brook_add_connection(con);

  return BROOK_OK;
}

int
brook_connection_read ( brook_connection_t* con ) {

  printf("Tenho dados para ler\n");

  // ... get buffer from chain or create if needed ...
  // ... check free buffer to write ...
  brook_buffer_chain_t* buffer = con->_data;
  brook_buffer_chain_t* last = NULL;

  while ( buffer != NULL && buffer->free == 0 ) {
    last = buffer;
    buffer = buffer->next;
  }

  if ( buffer == NULL ) {
    printf("Nao existe nenhum buffer\n");
    buffer = malloc(sizeof(brook_buffer_chain_t));
    buffer->data = malloc(4096);
    buffer->next = NULL;
    buffer->size = 4096;
    buffer->len = 0;
    buffer->free = 4096;

    if ( last == NULL ) {
      con->_data = buffer;
    } else {
      last->next = buffer;
    }
  }

  unsigned char* buf = buffer->data + buffer->len;

  size_t _n = brook_socket_recv(con->_fd, buf, buffer->free);
  if ( _n == 0 || _n == -1) {
    return BROOK_ERROR;
  }

  buffer->free -= _n;
  buffer->len += _n;
  printf("Foi lido %zu bytes\n", _n);

  // ... make http parse, consoant reading ...
  brook_http_parse(con->_parser, buf, _n);

  return BROOK_OK;
}

int
brook_add_connection ( brook_connection_t* con ) {

  // ... add to array of files descriptors to poll() of kernel ...
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if ( _fds[i].fd == -1 ) {
      struct pollfd* _fd = &_fds[i];
      _fd->fd = con->_fd;
      _fd->events = POLLIN;          // ... only add event of READING
      _connections[i] = con;        // ... this will make index 0 of array always empty
    }
  }

  return BROOK_OK;
}
