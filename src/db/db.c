//
//  db.c
//  http-c-broker
//
//  Created by David Xavier on 05/07/2025.
//

#include "db.h"

int init_db_connections (http_db_pool_struct* pool, http_str_s conn_str) {

    for (int i = 0; i < pool->conns_number; i++) {
         
        pool->conns[i]      = PQconnectdb(conn_str.data);
        pool->conns_info[i] = 0;
        
        // TODO: if exit, parent dont stop and will create more childrens
        if (PQstatus(pool->conns[i]) != CONNECTION_OK) {
            perror(PQerrorMessage(pool->conns[i]));
            exit(HTTP_ERROR);
        }
        
    }
    
    return HTTP_OK;
}

int get_db_connection (http_db_pool_struct* pool) {
    
    for (int i = 0; i < pool->conns_number; i++) {
        
        if (pool->conns_info[i] == 0) {
            pool->conns_info[i] = 1; // say connection is busy
            return i; // free conection
        }
        
    }
    
    return HTTP_NOT_OK;
}
