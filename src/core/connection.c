#include "core/connection.h"
#include "core/beanstalkd.h"
#include "http/http_request.h"
#include "http/http_response.h"
#include "http/http_session.h"

/**
 * Method calledd each time server socket contain connections waiting
 */
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
  if ( _socket == BROOK_DONE ) return BROOK_DONE;

  // ... create connection struct clean ...
  brook_connection_t* con = calloc(1, sizeof(brook_connection_t));
  if ( !con ) return BROOK_ERROR;

  // ... initial attributions ...
  con->_config = config;
  con->_fd    = _socket;
  con->_port  = ntohs(_addr.sin_port);
  inet_ntop(AF_INET, &(_addr.sin_addr), con->_ip, INET_ADDRSTRLEN);

  // ... init memory clean ...
  con->_parser = calloc(1, sizeof(brook_http_parse_t));

  if ( !con->_parser ) {
    free(con);
    return BROOK_OK;
  }

  // ... add conection to list ...
  if ( brook_add_connection(con) == BROOK_ERROR ) {
    // ... list is full ...
    free(con->_parser);
    free(con);
    return BROOK_ERROR;
  }

  return BROOK_OK;
}

/**
 * Method called each time socket contain data to read from
 */
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
  ssize_t _n = brook_socket_recv(con->_fd, buf, buffer->free);

  if ( _n == -1) {

    // ... some error append when socket reading ...
    brook_log(con->_config, LOG_ERR, "Socket receive error: %s (errno: %d) at %s:%d\n", strerror(errno), errno, __FILE__, __LINE__);
    brook_connection_reply(con, 500, (brook_str_t) brook_str("Internal Server Error"), (brook_str_t) brook_str("An error occurred while reading from the network socket. The stream may have been reset by the peer."));
    return BROOK_ERROR;

  } else if ( _n == 0 ) {

    // ... client disconnect socket ...
    brook_log(con->_config, LOG_DEBUG, "Connection closed by peer (client disconnected) at %s:%d\n", __FILE__, __LINE__);
    brook_destroy_connection(con);
    return BROOK_ERROR;

  }

  buffer->free -= (size_t) _n;
  buffer->len  += (size_t) _n;

  // ... make http parse, consoant data reading ...
  if ( brook_http_parse(con->_parser, buf, _n) == BROOK_ERROR ) {
    brook_connection_reply(con, 400, (brook_str_t) brook_str("Bad Request"), (brook_str_t) brook_str("The HTTP request could not be parsed due to invalid syntax or non-compliant headers."));
    return BROOK_ERROR;
  }

  // ... only make this logic when is parsing header ...
  if ( con->_parser->state < s_req_headers_done ) {
    // we have a problem, header is big than 4096 bytes

    // ... clear parser memory to retray ...
    memset(con->_parser, 0, sizeof(brook_http_parse_t));

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
      brook_connection_reply(con, 431, (brook_str_t) brook_str("Request Header Fields Too Large"), (brook_str_t) brook_str("he infrastructure rejected the request because the header section exceeds the configured size limit."));
      return BROOK_ERROR;
    }

    // ... realoc data of buffer ...
    buffer->data = realloc(d_old, buffer->size);

    // ... reset state of parser, to start over ...
    con->_parser->state = s_req_start;

    unsigned char* buf = buffer->data;

    // ... repeate process, to get new pointers in parser (parser need has memory in one sequencial array) ...
    if ( brook_http_parse(con->_parser, buf, buffer->len) == BROOK_ERROR ) {
      brook_connection_reply(con, 400, (brook_str_t) brook_str("Bad Request"), (brook_str_t) brook_str("The HTTP request could not be parsed due to invalid syntax or non-compliant headers."));
      return BROOK_ERROR;
    }

  } else {
    // ... check if i need read more data (body), because all header is parsed ...
    brook_http_parse_t* parser = con->_parser;

    // ... will only run 1 time, this will run after parse all http header ...
    if ( con->_role == NULL ) {
      // ... make validations of header request ...
      // ... validate gatekeeper ...
      brook_gatekeeper_node_t* route = brook_gatekeeper_match_route(con->_config->root, parser->url, parser->method);

      if ( route == NULL ) {
        brook_connection_reply(con, 404, (brook_str_t) brook_str("Resource Not Found"), (brook_str_t) brook_str("The server could not identify a route matching the provided path and method combination."));
        return BROOK_ERROR;
      } else {
        con->_role = route;
      }

      // ... check for size of request ...
      if ( parser->content_length > MAX_BODY_SIZE ) {
        brook_connection_reply(con, 413, (brook_str_t) brook_str("Payload Too Large"), (brook_str_t) brook_str("The request body exceeds the maximum size limit allowed by this server."));
        return BROOK_ERROR;
      }
    }

    // ... this can run multi times if body is bigger ...
    if ( parser->method == POST || parser->method == PUT || parser->method == PATCH ) {
      if ( parser->content_length > 0 ) {
        // need get data so check if is ok
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

  // ... check if request is parsed ...
  if ( con->_parser->state == s_req_done ) {

    // ... if its all ok s_req_done (request is done) we will create job payload ...
    con->_pfd->events &= ~POLLIN;

    // ... for now only validate request after receive all message ...
    if ( con->_role->role_mask > 0 ) {
      // ... session method, need validate session of user ...
      if ( brook_session_get_client_session(con) == BROOK_ERROR ) {
        return BROOK_ERROR;
      }
    } else {
      // ... public method ...
      if ( brook_benstalkd_create_job(con) == BROOK_ERROR ) {
        return BROOK_ERROR;
      }
    }

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
  con->_reponse.nwrite += b;

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

    brook_log(
      con->_config,
      LOG_INFO,
      "FD: %d IP: %s " \
      "Method: %s Url: %.*s%.*s " \
      "Content-Length: %d "
      "Access-Token: %.*s " \
      "Product Key: %.s " \
      "Status: %d Bytes: %d\n"
      ,
        con->_fd,
        con->_ip,

        brook_method_str[con->_parser->method],
        con->_parser->url.len,
        con->_parser->url.data,
        con->_parser->params_s.data ? (char*) con->_parser->params_s.len : 0,
        con->_parser->params_s.data ? (char*) con->_parser->params_s.data : "",

        con->_parser->content_length,

        con->session.token.data ? (char*) con->session.token.len : 0,
        con->session.token.data ? (char*) con->session.token.data : "",

        con->session.product_key,

        con->_reponse.status,
        con->_reponse.nwrite

    );

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

  if ( con->_parser->params != NULL ) {
    free(con->_parser->params);
  }

  // ... free http parser ...
  free(con->_parser);

  // ... at least free con ...
  free(con);

  con->_data = NULL;
  con->_parser = NULL;
  con = NULL;

  return BROOK_OK;
}


