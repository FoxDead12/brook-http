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

int get_db_query_result (http_connection_struct* con, int socket) {
    
    PGconn* db;
    PGresult *res;
    
    get_db_connection_from_socket(&con->worker->pool, socket, &db);
    
    // create response body to generate the json api template
    http_str_s* resource    = &con->response.json_api.resource;
    http_str_s* resource_id = &con->response.json_api.resource_id;
    con->response.json_api.b = json_object_new_object();
    
    json_object* body = con->response.json_api.b;
    
    if (resource_id->length <= 0) {
        json_object_object_add(body, "data", json_object_new_array());
    }
    
    while ((res = PQgetResult(db)) != NULL) {
        
        ExecStatusType status = PQresultStatus(res);
        printf("Status: %s\n", PQresStatus(status));

        int nrows = PQntuples(res);
        int ncols = PQnfields(res);

        if (nrows > 0) {
            if (resource_id->length > 0) {
                json_object* row = json_object_new_object();
                db_result_parse_row(res, row, resource, 0, ncols);
                json_object_object_add(body, "data", row);
            } else {
                json_object* data = json_object_object_get(body, "data");
                for (int i = 0; i < nrows; i++) {
                    json_object* row = json_object_new_object();
                    db_result_parse_row(res, row, resource, i, ncols);
                    json_object_array_add(data, row);
                }
            }
        }
        
        PQclear(res);
    }

    free_db_connection(&con->worker->pool, socket);
    
    return HTTP_OK;
}

int db_result_parse_row (PGresult* result, json_object* obj, http_str_s* resource, int row, int column_num) {
    
    json_object_object_add(obj, "type", json_object_new_string_len(resource->data, (int) resource->length));
    json_object* attributes = json_object_new_object();
    
    for (int c = 0; c < column_num; c++) {

        char* key   = PQfname(result, c);
        char* value = PQgetvalue(result, row, c);
        
        if (strcmp(key, "id") == 0) {
            json_object_object_add(obj, "id", json_object_new_string(value));
        } else {
            json_object_object_add(attributes, key, json_object_new_string(value));
        }
        
    }

    json_object_object_add(obj, "attributes", attributes);
    
    return HTTP_OK;
}
