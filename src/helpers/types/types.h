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

typedef struct {
    char* data;
    int length;
} http_str_s;

typedef struct {
    http_str_s key;
    http_str_s value;
} http_table_s;

typedef struct {
    char* start;
    char* end;
    int lenght;
} http_buffer_s;

typedef struct {
    char *data;
    int length;
    struct http_chain_s *next;
} http_chain_s;

typedef struct {
    pid_t pid;
} http_worker_struct;

typedef struct {
    pid_t pid;
    json_object *conf;
    int socket;
    int port;
    int worker_processes;
    http_worker_struct *workers;
} http_main_struct;

typedef struct {
    http_buffer_s data;
    http_str_s method;
    http_str_s url;
    http_str_s host;
    http_str_s connection;
    int content_length;
    http_str_s content_type;
} http_request_header_struct;

typedef struct {
    int socket;
    http_main_struct *server_config;
    http_request_header_struct header;
    http_chain_s body;
} http_request_struct;

http_str_s http_str (char* data);
int conv_str_to_int (http_str_s s);
int comp_str_to_str (http_str_s a, http_str_s b);

#endif /* types_h */
