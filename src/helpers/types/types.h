//
//  types.h
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#ifndef types_h
#define types_h

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <json-c/json.h>
#include <string.h>
#include <arpa/inet.h>
#include <libpq-fe.h>
#include <regex.h>


#define HTTP_NOT_OK     -1
#define HTTP_OK         0      // Server sucess
#define HTTP_ERROR      1      // Server error
#define HTTP_DONE       2      // Server need await for something


typedef struct http_str_s       http_str_s;
typedef struct http_table_s     http_table_s;
typedef struct http_buffer_s    http_buffer_s;
typedef struct http_chain_s     http_chain_s;


typedef struct http_main_struct            http_main_struct;
typedef struct http_worker_struct          http_worker_struct;
typedef struct http_connection_struct      http_connection_struct;
typedef struct http_request_headers_struct http_request_headers_struct;
typedef struct http_request_struct         http_request_struct;
typedef struct http_db_pool_struct         http_db_pool_struct;
typedef struct http_response_struct        http_response_struct;
typedef struct http_json_api_struct        http_json_api_struct;

struct http_str_s {
    char*       data;
    size_t      length;
};

struct http_table_s {
    http_str_s  key;
    http_str_s  value;
};

struct http_buffer_s {
    char*       start;
    char*       end;
    size_t      length;
    size_t      size;
};

struct http_chain_s {
    http_buffer_s buffer;
    http_chain_s* next;
};




struct http_db_pool_struct {
    PGconn**        conns;
    int             conns_number;
    int*            conns_info;
};




struct http_main_struct {
    pid_t               pid;
    json_object*        conf;
    int                 socket;
    int                 port;
    int                 worker_processes;
    http_worker_struct* workers; // only parent process will contain this array
    regex_t             regex_header;
    regex_t             regex_url;
};

struct http_worker_struct {
    pid_t                pid;
    http_main_struct*    server;
    http_db_pool_struct  pool;
};

struct http_request_headers_struct {
    http_str_s                 host;
    http_str_s                 connection;
    http_str_s                 content_type;
    int                        content_length;
};

struct http_request_struct {
    http_str_s                 method;
    http_str_s                 url;
    http_str_s                 params;
    http_request_headers_struct headers;
};

struct http_json_api_struct {
    http_str_s              resource;
    http_str_s              resource_id;
    json_object*            b;
};

struct http_response_struct {
    http_json_api_struct    json_api;
};

struct http_connection_struct {
    char                       ip[INET_ADDRSTRLEN];
    int                        port;
    int                        socket;
    http_buffer_s              b_header;
    http_chain_s               c_body;
    http_request_struct        request;
    http_response_struct       response;
    http_worker_struct*        worker;
    int                        status; // 0 - read data from socket, 1 - build query, 2 - Get response from database
};



http_str_s http_str        (char* data);
int        str_to_int      (http_str_s s);
int        comp_str_to_str (http_str_s a, http_str_s b);

#endif /* types_h */
