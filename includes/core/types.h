#ifndef _BROOK_TYPES_H
#define  _BROOK_TYPES_H

#include <stdio.h>
#include <stdlib.h>

#define brook_str(str) { sizeof(str) - 1, (unsigned char*) str }

typedef enum {
  DELETE,
  GET,
  POST,
  PUT,
  PATCH
} brook_method_e;

static const char *brook_method_str[] = {
  [DELETE] = "DELETE",
  [GET]    = "GET",
  [POST]   = "POST",
  [PUT]    = "PUT",
  [PATCH]  = "PATCH"
};

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
  size_t nread;     // ... only will use when is reading from buffer
  unsigned char* data;
  struct brook_buffer_chain_s* next;

} brook_buffer_chain_t;

typedef struct brook_gatekeeper_node_s {
  brook_str_t url;
  brook_str_t tube;
  uint32_t methods_mask;

  uint32_t    role_mask;
  brook_str_t product_key;

  struct brook_gatekeeper_node_s* left;
  struct brook_gatekeeper_node_s* rigth;
} brook_gatekeeper_node_t;

typedef struct {

  // ... configs load from config file ...
  char name[32];
  int  port;
  int  workers;
  char gatekeeper[256];
  char log[1024];
  struct {
    char host[64];
    int  port;
  } beanstalkd;
  struct {
    char host[64];
    int  port;
  } redis;

  // ... internal configs ...
  int socket;
  brook_gatekeeper_node_t* root;
} brook_conf_t;

typedef struct {
  brook_str_t key;
  brook_str_t value;
} brook_params_t;

typedef struct {
  unsigned char state;          // ... state of parse
  unsigned short header_state;  // ... state of header parse

  unsigned short http_minor;  // ... version of http
  unsigned short method;      // ... method of http request [brook_method_e]

  unsigned short index;       // ... used to keep flow in multi bytes validate

  uint64_t nheader;
  uint64_t nread;             // ... bytes already read/already check
  uint64_t content_length;

  brook_str_t url;
  brook_str_t cookies;

  brook_str_t params_s;
  brook_params_t* params;
  int params_n;
  int params_capacity;


} brook_http_parse_t;

typedef struct {
  uint16_t status;
  uint64_t nwrite;             // ... bytes already write/sended check
  brook_buffer_chain_t* _data;

} brook_http_response_t;

typedef struct {
  unsigned char state;

  uint64_t id;

  uint32_t priority;
  uint32_t ttr;
  uint32_t delay;

  uint32_t retray;
  uint32_t max_retray;

  brook_str_t tube;
  brook_str_t data;

} brook_job_t;

typedef struct {
  brook_str_t   token;
  uint32_t      role_mask;
  char          schema[32];
  char          product_key[32];
  int           user_id;
} brook_session_t;

// ... struct to define the connection struct, will handle all necessary data to manager a connection ...
typedef struct {
  struct pollfd* _pfd;
  brook_conf_t* _config;

  int   _fd;              // ... is the socket
  int   _status;
  int   _port;
  char  _ip[INET_ADDRSTRLEN];

  brook_buffer_chain_t*    _data;
  brook_http_parse_t*      _parser;
  brook_http_response_t    _reponse;
  brook_gatekeeper_node_t* _role;

  brook_session_t session;
  brook_job_t     job;

} brook_connection_t;

#endif
