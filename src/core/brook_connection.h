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
    READING_SOCKET_MESSAGE
};

struct brook_connection_s {
    brook_config_t* conf;
    brook_connection_status_t state;
    pid_t socket;
    int port;
    char ip[INET_ADDRSTRLEN];
};

brook_connection_t* brook_create_connection(brook_config_t* conf);
int brook_read_message_connection(brook_connection_t* connection);

#endif /* brook_connection_h */
