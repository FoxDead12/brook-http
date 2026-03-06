#ifndef _BROOK_TYPES_H
#define  _BROOK_TYPES_H

#include <stdio.h>
#include <stdlib.h>

#define brook_str(str)    { sizeof(str) - 1, (u_char *) str }

typedef enum {
  DELETE,
  GET,
  POST,
  PUT,
  PATCH
} brook_method_e;

// ... struct to define a "string" will be used to buffers read ...
typedef struct {
  size_t len;
  unsigned char *data;
} brook_str_t;

typedef struct brook_node_s {
  void* data;
  struct brook_node_s* next;
} brook_node_t;

typedef struct {
  size_t size;      // ... memory alloced to buffer
  size_t len;       // ... current memory used in buffer
  size_t free;      // ... memory free to fill all buffer
  unsigned char* data;
} brook_buffer_t;

typedef struct brook_buffer_chain_s {
  size_t size;      // ... memory alloced to buffer
  size_t len;       // ... current memory used in buffer
  size_t free;      // ... memory free to fill all buffer
  unsigned char* data;
  struct brook_buffer_chain_s* next;

} brook_buffer_chain_t;

typedef struct brook_gatekeeper_node_s {
  brook_str_t url;
  // ... jobs options ...
  brook_str_t tube;
  uint32_t methods_mask;
  struct brook_gatekeeper_node_s* left;
  struct brook_gatekeeper_node_s* rigth;
} brook_gatekeeper_node_t;

typedef struct {
  int socket;
  brook_gatekeeper_node_t* root;
} brook_conf_t;

typedef struct {
  unsigned char state;          // ... state of parse
  unsigned short header_state;  // ... state of header parse

  unsigned short http_minor;  // ... version of http
  unsigned short method;      // ... method of http request [brook_method_e]

  unsigned short index;       // ... used to keep flow in multi bytes validate

  uint64_t nread;             // ... bytes already read/already check
  uint64_t content_length;

  brook_str_t url;
  brook_str_t params;

} brook_http_parse_t;

typedef struct {
  uint64_t nwrite;             // ... bytes already write/sended check
  brook_str_t data;            // ... pointer to buffer of all response message
} brook_http_response_t;


// ... struct to define the connection struct, will handle all necessary data to manager a connection ...
typedef struct {
  struct pollfd* _pfd;
  brook_conf_t* _config;

  int   _fd;              // ... is the socket
  int   _status;
  int   _port;
  char  _ip[INET_ADDRSTRLEN];

  brook_buffer_chain_t* _data;
  brook_http_parse_t* _parser;

  brook_gatekeeper_node_t* role;

  brook_http_response_t reponse;

} brook_connection_t;

#endif
