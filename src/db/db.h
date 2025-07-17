//
//  db.h
//  http-c-broker
//
//  Created by David Xavier on 05/07/2025.
//

#ifndef db_h
#define db_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libpq-fe.h>
#include <string.h>
#include <errno.h>
#include "../helpers/types/types.h"

int init_db_connections           (http_db_pool_struct* pool, http_str_s conn_str);
int free_db_connection            (http_db_pool_struct* pool, int socket);
int get_db_free_connection        (http_db_pool_struct* pool);
int get_db_connection_from_socket (http_db_pool_struct* pool, int socket, PGconn** db);
int get_db_query_result           (http_connection_struct* con, int socket);
int db_result_parse_row           (PGresult* result, json_object* obj, http_str_s* resource, int row, int column_num);

#endif /* db_h */
