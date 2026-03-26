#include "core/connection.h"
#include "core/beanstalkd.h"
#include "http/http_request.h"
#include "http/http_response.h"

int
brook_handle_connection ( brook_conf_t* config) {

  if ( CURRENT_FD >= MAX_FD ) {
    return BROOK_ERROR;
  }

  // ... accept file descriptor connection ...
  struct sockaddr_in _addr;
  socklen_t addr_len = sizeof(_addr);

  // ... accept connection ...
  int _socket = brook_socket_accept(config->socket, (struct sockaddr*) &_addr, &addr_len);
  if ( _socket == BROOK_DONE ) {
    return BROOK_DONE;
  }

  // printf("Nova conexao estabelecida\n");

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

  con->_reponse._data = NULL;
  con->_reponse.nwrite = 0;
  con->_reponse.status = 0;

  con->job.data.data = NULL;
  con->job.tube.data = NULL;

  // ... add conection to list ...
  brook_add_connection(con);

  return BROOK_OK;
}

int
brook_connection_read ( brook_connection_t* con ) {

  // ... get buffer from chain or create if needed ...
  // ... check free buffer to write ...
  brook_buffer_chain_t* buffer = con->_data;
  brook_buffer_chain_t* last = NULL;

  while ( buffer != NULL && buffer->free == 0 ) {
    last = buffer;
    buffer = buffer->next;
  }

  if ( buffer == NULL ) {
    buffer = malloc(sizeof(brook_buffer_chain_t));
    buffer->data = malloc(4096);
    buffer->next = NULL;
    buffer->size = 4096;
    buffer->len = 0;
    buffer->nread = 0;
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
    brook_connection_reply(con, 500, (brook_str_t) brook_str("Network Read Error"), (brook_str_t) brook_str("Interrupted system call or connection reset during recv operation. Check network stability."));
    return BROOK_ERROR;
  }

  buffer->free -= _n;
  buffer->len += _n;

  // ... make http parse, consoant reading ...
  if ( brook_http_parse(con->_parser, buf, _n) == BROOK_ERROR ) {
    brook_connection_reply(con, 400, (brook_str_t) brook_str("Malformed HTTP Request"), (brook_str_t) brook_str("The request syntax is invalid or contains non-compliant characters. Please verify the protocol headers and body structure."));
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

    // ... limit header buffer max size (ex: 12500) a sanity check todo ...
    if ( buffer->size > 12288 ) {
      brook_connection_reply(con, 400, (brook_str_t) brook_str("Header Too Big"), (brook_str_t) brook_str("The request header exceeds the maximum allowed size."));
      return BROOK_ERROR;
    }

    // ... realoc data of buffer ...
    buffer->data = realloc(d_old, buffer->size);

    // ... reset state of parser, to start over ...
    con->_parser->state = s_req_start;

    unsigned char* buf = buffer->data;

    // ... repeate process, to get new pointers in parser (parser need has memory in one sequencial array) ...
    if ( brook_http_parse(con->_parser, buf, buffer->len) == BROOK_ERROR ) {
      brook_connection_reply(con, 400, (brook_str_t) brook_str("Malformed HTTP Request"), (brook_str_t) brook_str("The request syntax is invalid or contains non-compliant characters. Please verify the protocol headers and body structure."));
      return BROOK_ERROR;
    }

  } else {
    // ... check if i need read more data (body), because all header is parsed ...
    brook_http_parse_t* parser = con->_parser;

    // ... make validations of header request ...
    // ... validate gatekeeper ...
    brook_gatekeeper_node_t* route = brook_gatekeeper_match_route(con->_config->root, parser->url, parser->method);
    if ( route == NULL ) {
      brook_connection_reply(con, 404, (brook_str_t) brook_str("Resource Not Found"), (brook_str_t) brook_str("The requested endpoint does not exist. Please verify the URL path and the HTTP method used."));
      return BROOK_ERROR;
    } else {
      con->_role = route;
    }

    if ( parser->method == POST || parser->method == PUT || parser->method == PATCH ) {
      if ( parser->content_length > 0 ) {
        // need get data so check if is ok

        if ( parser->content_length > MAX_BODY_SIZE ) {
          brook_connection_reply(con, 413, (brook_str_t) brook_str("Payload Too Large"), (brook_str_t) brook_str("The request body exceeds the maximum size limit allowed by this server."));
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
    con->_pfd->events &= ~POLLIN;
    brook_benstalkd_create_job(con);
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
  con->_pfd->events = POLLOUT;      // ... change events of poll socket
  return BROOK_OK;
}

int
brook_connection_write ( brook_connection_t* con ) {

  unsigned char* buf = con->_reponse._data->data + con->_reponse._data->nread;
  size_t len = con->_reponse._data->len - con->_reponse._data->nread;

  size_t b = send(con->_fd, buf, len, 0);
  con->_reponse._data->nread += b;

  // ... if we dont send all buffer will return to send the next of buffer
  if ( con->_reponse._data->len != con->_reponse._data->nread ) {
    return BROOK_DONE;
  }

  brook_buffer_chain_t* buffer = con->_reponse._data;
  if ( buffer->next != NULL ) {
    con->_reponse._data = buffer->next;
    free(buffer->data);
    free(buffer);
    return BROOK_DONE;
  } else {
    brook_log(con->_config, LOG_INFO, " %s \"%s %.*s\" %d\n", con->_ip, brook_method_str[con->_parser->method], con->_parser->url.len, con->_parser->url.data, con->_reponse.status);
    free(buffer->data);
    free(buffer);
    brook_destroy_connection(con);
    return BROOK_OK;
  }
}

int
brook_destroy_connection ( brook_connection_t* con ) {

  // ... remove pointer from main array in memory ...
  for (int i = 0; i <= CURRENT_FD; i++) {
    if (_connections[i] == con) {
      _connections[i] = NULL;
      break;
    }
  }

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

  // ... free job ...
  if ( con->job.data.data != NULL ) {
    free(con->job.data.data);
  }

  // ... free http parser ...
  free(con->_parser);

  // ... at least free con ...
  free(con);

  return BROOK_OK;
}


