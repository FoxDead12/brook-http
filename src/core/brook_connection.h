//
//  brook_connection.h
//  http-c-broker
//
//  Created by David Xavier on 29/07/2025.
//

#ifndef brook_connection_h
#define brook_connection_h

#include "brook_config.h"

typedef struct brook_http_s brook_http_t;
typedef struct brook_connection_s brook_connection_t;
typedef enum   brook_connection_status_e brook_connection_status_t;

enum brook_connection_status_e {
	READING_SOCKET_MESSAGE,
	READING_REDIS_MESSAGE,
	READING_PSQL_MESSAGE,

	WAITING_POOL_DB,
    WAITING_POOL_REDIS,

	WRITING_SOCKET_MESSAGE,
	WRITING_REDIS_MESSAGE,
    WRITING_BEANSTALK_MESSAGE,
	WRITING_PSQL_MESSAGE
};

struct brook_connection_s {
    brook_config_t*           conf;
    brook_connection_status_t state;
    pid_t                     socket;
    int                       port;
    char                      ip[INET_ADDRSTRLEN];
    brook_chain_t*            ch_buf;
	brook_chain_t*            pos;
    brook_http_t*             http;
};

brook_connection_t* brook_create_connection(brook_config_t* conf);
int brook_read_message_connection(brook_connection_t* connection);
int brook_close_connection(brook_connection_t* connection);
int brook_connection_write_psql(brook_connection_t* connection, int pg_socket);

#endif /* brook_connection_h */
