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


int free_db_connection (http_db_pool_struct* pool, int socket) {
    
    for (int i = 0; i < pool->conns_number; i++) {
        int s = PQsocket(pool->conns[i]);
        
        if (s == socket) {
            pool->conns_info[i] = 0;
        }
    }
    
    return HTTP_OK;
}

int get_db_free_connection (http_db_pool_struct* pool) {
    
    for (int i = 0; i < pool->conns_number; i++) {
        
        if (pool->conns_info[i] == 0) {
            pool->conns_info[i] = 1; // say connection is busy
            return i; // free conection
        }
        
    }
    
    return HTTP_NOT_OK;
}

int get_db_connection_from_socket (http_db_pool_struct* pool, int socket, PGconn** db) {
    
    for (int i = 0; i < pool->conns_number; i++) {
        
        int s = PQsocket(pool->conns[i]);
        
        if (s == socket) {
            *db = pool->conns[i];
            return HTTP_OK;
        }
        
    }
    
    *db = NULL;
    
    return HTTP_OK;
}

int get_db_query_result (http_connection_struct*    con, int socket) {
    
    PGconn* db;
    get_db_connection_from_socket(&con->worker->pool, socket, &db);
    
    PGresult *res;
    while ((res = PQgetResult(db)) != NULL) {
        
        ExecStatusType status = PQresultStatus(res);
        
        int nrows = PQntuples(res);
        int ncols = PQnfields(res);

        for (int i = 0; i < nrows; i++) {
            for (int j = 0; j < ncols; j++) {
                printf("%s\t", PQgetvalue(res, i, j));
            }
            printf("\n");
        }
        
        PQclear(res);
    }

    free_db_connection(&con->worker->pool, socket);
    
    return HTTP_OK;
}
