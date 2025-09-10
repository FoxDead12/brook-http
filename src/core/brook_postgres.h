//
//  brook_postgres.h
//  http-c-broker
//
//  Created by David Xavier on 26/08/2025.
//

#ifndef brook_postgres_h
#define brook_postgres_h

#include "brook_config.h"

typedef struct brook_postgres_s brook_postgres_t;
typedef enum brook_postgres_connection_e brook_postgres_connection_t;


enum brook_postgres_connection_e {
	READY,
	OCUPPIED
};

struct brook_postgres_s {
    PGconn** conns;
	int*	 state;
};

int brook_postgres_connections_init(brook_config_t* config);
PGconn* brook_postgres_get_connection(brook_config_t* config);
PGconn* brook_postgres_get_connection_from_socket(brook_config_t* config, int pg_socket);
int brook_postgres_free_connection(brook_config_t* config, int socket);


#endif /* brook_postgres_h */
