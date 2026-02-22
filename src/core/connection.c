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
  con->_parser->header_state = 0;

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

  // ... make http parse, consoant reading ...
  if ( brook_http_parse(con->_parser, buf, _n) == BROOK_ERROR ) {
    return BROOK_ERROR;
  }

  // ... only make this logic when is parsing header ...
  if ( con->_parser->state < s_req_headers_done ) {
    // we have a problem, header is big than 4096 bytes
    // so will realoc buffer to a bigger size
    size_t size_to_sum = 4096;

    // ... store old buffer information ...
    unsigned char* d_old = buffer->data;
    size_t s_old = buffer->size;

    // ... recalculate new space ...
    buffer->size = s_old + size_to_sum;
    buffer->free = buffer->size - buffer->len;

    // ... TODO: limit header size (ex. 12500) a sanity check todo...

    // ... realoc data of buffer ...
    buffer->data = realloc(d_old, buffer->size);

    // ... reset state of parser, to start over ...
    con->_parser->state = s_req_start;

    unsigned char* buf = buffer->data;

    // ... repeate process, to get new pointers in parser (parser need has memory in one sequencial array) ...
    if ( brook_http_parse(con->_parser, buf, buffer->len) == BROOK_ERROR ) {
      return BROOK_ERROR;
    }

  } else {

    // ... check if i need read more data (body), because all header is parsed ...
    brook_http_parse_t* parser = con->_parser;

    if ( parser->method == POST || parser->method == PUT ) {
      if ( parser->content_length > 0 ) {
        // need get data so check if is ok

        if ( parser->content_length > MAX_BODY_SIZE ) {
          printf("body is to big\n");
          return BROOK_ERROR;
        }

        if ( parser->nread < parser->content_length ) {
          parser->state = s_req_body;
        } else {
          parser->state = s_req_done;
        }

      } else {
        // its all parsed, now flow to send job
        parser->state = s_req_done;
      }
    } else {
      parser->state = s_req_done;
    }
  }

  if ( con->_parser->state == s_req_done ) {
    return BROOK_OK; // parser is finish
  } else {
    return BROOK_DONE; // parser is finish
  }
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



