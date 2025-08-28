//
//  brook_postgres.c
//  http-c-broker
//
//  Created by David Xavier on 26/08/2025.
//

#include "brook_postgres.h"
#include "brook_core.h"

int
brook_postgres_connections_init (brook_config_t* config) {
    
	char* string = (char*) json_get_str("postgres_connection_string", config->json, (brook_str_t) brook_string("")).data;
	
	config->postgres_conns->conns = malloc(sizeof(PGconn*) * config->postgres_con_worker);
	config->postgres_conns->state = malloc(sizeof(int) * config->postgres_con_worker);
	
	for (int i = 0; i < config->postgres_con_worker; i++) {
		config->postgres_conns->conns[i] = PQconnectdb(string);
		config->postgres_conns->state[i] = READY;
		
		if (PQstatus(config->postgres_conns->conns[i]) != CONNECTION_OK) {
		   perror(PQerrorMessage(config->postgres_conns->conns[i]));
			return BROOK_ERROR;
	   }
	}
    
    return BROOK_OK;
}

PGconn*
brook_postgres_get_connection (brook_config_t* config) {
	
	for (int i = 0; i < config->postgres_con_worker; i++) {
		if (config->postgres_conns->state[i] == READY) {
			config->postgres_conns->state[i] = OCUPPIED;
			return config->postgres_conns->conns[i];
		}
	}
	
	return NULL;
}

PGconn*
brook_postgres_get_connection_from_socket (brook_config_t* config, int pg_socket) {
	
	for (int i = 0; i < config->postgres_con_worker; i++) {
		PGconn* db = config->postgres_conns->conns[i];
		if (PQsocket(db) == pg_socket) {
			return db;
		}
	}
	
	return NULL;
}
