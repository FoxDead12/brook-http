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
            exit(0);
        }
        
    }
    
    return 0;
}
