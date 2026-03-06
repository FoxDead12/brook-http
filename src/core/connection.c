#include "core/connection.h"
#include "http/http_request.h"
#include "http/http_response.h"

int
brook_handle_connection ( brook_conf_t* config) {
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

  con->reponse.data.data = NULL;
  con->reponse.data.len = 0;
  con->reponse.nwrite = 0;

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
    brook_connection_reply(con, 500, (brook_str_t) brook_str("Can't read data from socket"), (brook_str_t) brook_str("Can't read data from socket"));
    return BROOK_ERROR;
  }

  buffer->free -= _n;
  buffer->len += _n;

  // ... make http parse, consoant reading ...
  if ( brook_http_parse(con->_parser, buf, _n) == BROOK_ERROR ) {
    brook_connection_reply(con, 400, (brook_str_t) brook_str("Invalid HTTP message"), (brook_str_t) brook_str("Probably send invalid format or invalid bit"));
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

    // ... TODO: limit header buffer max size (ex: 12500) a sanity check todo ...

    // ... realoc data of buffer ...
    buffer->data = realloc(d_old, buffer->size);

    // ... reset state of parser, to start over ...
    con->_parser->state = s_req_start;

    unsigned char* buf = buffer->data;

    // ... repeate process, to get new pointers in parser (parser need has memory in one sequencial array) ...
    if ( brook_http_parse(con->_parser, buf, buffer->len) == BROOK_ERROR ) {
      brook_connection_reply(con, 400, (brook_str_t) brook_str("Invalid HTTP message"), (brook_str_t) brook_str("Probably send invalid format or invalid bit"));
      return BROOK_ERROR;
    }

  } else {
    // ... check if i need read more data (body), because all header is parsed ...
    brook_http_parse_t* parser = con->_parser;

    // ... make validations of header request ...
    // ... validate gatekeeper ...
    brook_gatekeeper_node_t* route = brook_gatekeeper_match_route(con->_config->root, parser->url, parser->method);
    if ( route == NULL ) {
      brook_connection_reply(con, 404, (brook_str_t) brook_str("Route not found"), (brook_str_t) brook_str("Invalid route check url and method"));
      return BROOK_ERROR;
    } else {
      con->role = route;
    }

    if ( parser->method == POST || parser->method == PUT || parser->method == PATCH ) {
      if ( parser->content_length > 0 ) {
        // need get data so check if is ok

        if ( parser->content_length > MAX_BODY_SIZE ) {
          brook_connection_reply(con, 413, (brook_str_t) brook_str("Body is too large"), (brook_str_t) brook_str("Your body pass the limit of server body max size"));
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
    // ... if its all ok s_req_done (request is done) we will create job payload ...
    return BROOK_OK; // parser is finish
  } else {
    return BROOK_DONE; // parser is finish
  }
}

int
brook_add_connection ( brook_connection_t* con ) {
  // ... add to array of files descriptors to poll() of kernel ...
  for (int i = 0; i < MAX_FD; i++) {
    if ( _fds[i].fd == -1 ) {
      struct pollfd* _fd = &_fds[i];
      _fd->fd = con->_fd;
      _fd->events = POLLIN;          // ... only add event of READING

      con->_pfd = &_fds[i];
      _connections[i] = con;        // ... this will make index 0 of array always empty

      CURRENT_FD = i > CURRENT_FD ? i : CURRENT_FD;       // TODO: this need be analyze because number will never down
      return BROOK_OK;
    }
  }

  return BROOK_ERROR;
}

int
brook_connection_reply ( brook_connection_t* con, uint16_t code, brook_str_t message, brook_str_t detail ) {
  brook_http_response_static(con, code, message, detail);
  con->_pfd->events = POLLOUT;
  return BROOK_OK;
}

int
brook_connection_write ( brook_connection_t* con ) {
  char* p = con->reponse.data.data + con->reponse.nwrite;
  uint64_t b = con->reponse.data.len - con->reponse.nwrite;
  con->reponse.nwrite += send(con->_fd, p, b, 0);

  if ( con->reponse.nwrite >= con->reponse.data.len ) {
    brook_destroy_connection(con);
    return BROOK_OK;
  }

  return BROOK_DONE;
}

int
brook_destroy_connection ( brook_connection_t* con ) {

  // ... close socket ...
  close(con->_fd);

  // ... clean fd struct ...
  struct pollfd* _fd = con->_pfd;
  _fd->fd = -1;
  _fd->events = 0;
  _fd->revents = 0;

  while (CURRENT_FD > 0 && _fds[CURRENT_FD].fd == -1) {
    CURRENT_FD--;
  }

  // ... free memory buffers ...
  brook_buffer_chain_t* buffer = con->_data;
  brook_buffer_chain_t* tmp = NULL;

  while ( buffer != NULL ) {
    tmp = buffer;
    buffer = buffer->next;

    free(tmp->data);
    free(tmp);
  }

  if ( con->reponse.data.data ) {
    free(con->reponse.data.data);
  }

  // ... free http parser ...
  free(con->_parser);

  // ... at least free con ...
  free(con);

  return BROOK_OK;
}


