//
//  brook_connection.h
//  http-c-broker
//
//  Created by David Xavier on 29/07/2025.
//

#ifndef brook_connection_h
#define brook_connection_h

#include "brook_core.h"

typedef struct brook_connection_s brook_connection_t;
typedef enum brook_connection_status_e brook_connection_status_t;

enum brook_connection_status_e {
    READING_REQUEST_HEADER,
    READING_REQUEST_BODY,
};

struct brook_connection_s {
    brook_config_t* conf;
    pid_t socket;
    char ip[INET_ADDRSTRLEN];
    int port;
    brook_connection_status_t state;
};

brook_connection_t* brook_create_connection(brook_config_t* conf);

#endif /* brook_connection_h */
