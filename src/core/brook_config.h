//
//  brook_config.h
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#ifndef brook_config_h
#define brook_config_h

#include <stdio.h>
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

typedef unsigned char u_char;

#include "brook_array.h"
#include "brook_string.h"

typedef struct brook_config_s      brook_config_t;
typedef struct brook_config_http_s brook_config_http_t;
typedef struct brook_array_s       brook_array_t;

struct brook_config_http_s {
    size_t timeout;
    size_t max_body_size;
    brook_array_t* allow_content_types;
};

struct brook_config_s {
    json_object* json;
    int port;
    int worker_processes;
    int socket;
    brook_config_http_t http;
};

#endif /* brook_config_h */
