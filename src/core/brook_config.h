//
//  brook_config.h
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#ifndef brook_config_h
#define brook_config_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <json-c/json.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <regex.h>
#include <dirent.h>
#include <libpq-fe.h>

typedef unsigned char u_char;

#include "brook_array.h"
#include "brook_string.h"
#include "brook_buffer.h"

typedef struct brook_config_s           brook_config_t;
typedef struct brook_config_http_s      brook_config_http_t;
typedef struct brook_array_s            brook_array_t;
typedef struct brook_config_processes_s brook_config_processes_t;
typedef struct brook_config_regex_s     brook_config_regex_t;
typedef struct brook_postgres_s brook_postgres_t;

struct brook_config_regex_s {
    regex_t http_line;
};

struct brook_config_processes_s {
    pid_t pid;
};

struct brook_config_http_s {
    size_t timeout;
    size_t max_body_size;
    size_t buffers_size;
};

struct brook_config_s {
    json_object* json;
    int port;
    int worker_processes;
    int socket;
    pid_t                    brook_parent_process;
    brook_config_http_t      http;
    brook_array_t*           brook_processes;
    brook_config_processes_t brook_process;
    brook_config_regex_t     regex;
    json_object*             resources;
    brook_array_t*           gatekeeper;
	
	// ... database configurations ...
	brook_postgres_t*		 postgres_conns;
	int postgres_con_worker;
};

#endif /* brook_config_h */
