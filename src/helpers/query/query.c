//
//  query.c
//  http-c-broker
//
//  Created by David Xavier on 13/07/2025.
//

#include "query.h"

int generate_query_from_request (http_connection_struct* con, PGconn* db) {
    
    /*
    printf("METHOD: %.*s\n", con->request.method.length, con->request.method.data);
    printf("URL: %.*s\n",    con->request.url.length, con->request.url.data);
    printf("PARAMS: %.*s\n", con->request.params.length, con->request.params.data);
    printf("CONTENT_TYPE: %.*s\n",    con->request.headers.content_type.length, con->request.headers.content_type.data);
    */
    
    /*
   
     What is needed:
        resource    -> table
        resource_id -> id if necessary
        body  -> json payload (but we dont has body wet)
     
     
    Querys possiveis, e neste momento apenas executamos uma query por pedido (TODO: permitir multi querys)

    const char* query = "SELECT * FROM @resource";
    
    const char* query = "SELECT * FROM @resource WHERE id = @resource_id";
    
    const char* query = "INSERT INTO @resource (@resource_param1, @resource_param2, @resource_param3, @resource_param4) VALUES (@resource_value1, @resource_value2, @resource_value3, @resource_value4)";
    
    const char* query = "UPDATE @resource SET @resource_param1 = @resource_value1, @resource_param2 = @resource_value2 WHERE id = @resource_id";
    
    const char* query = "DELETE FROM @resource WHERE id = @resource_id";
    
    */
    
    http_str_s query;
     
    // Get resource data from request
    if (parse_resource_data(con, &con->response.json_api.resource, &con->response.json_api.resource_id) == HTTP_ERROR) {
        return HTTP_ERROR;
    }
    
    http_str_s* resource    = &con->response.json_api.resource;
    http_str_s* resource_id = &con->response.json_api.resource_id;
            
    // Generate query from template
    if (comp_str_to_str(con->request.method, http_str("GET")) == 0)  {
        
        if (resource_id->length > 0) {
            query = select_item_query(resource, resource_id);
        } else {
            query = select_query(resource);
        }
        
    } else if (comp_str_to_str(con->request.method, http_str("DELETE")) == 0) {
        
        if (resource_id->length <= 0) {
            send_json_api_response_error(con, 400, "HTTP_BROKER_ERROR_ATTRIBUTE", "Is necessary ID in resource data");
            return HTTP_ERROR;
        }
        
        query = delete_item_query(resource, resource_id);
    }
    
    // Send query to database
    PQsendQuery(db, query.data);
    
    // Free query
    free(query.data);
    
    return HTTP_OK;
}

int parse_resource_data (http_connection_struct* con, http_str_s* resource, http_str_s* resource_id) {
    
    regmatch_t matches[3];
    int reg = regexec(&con->worker->server->regex_url, con->request.url.data, 3, matches, 0);
    
    if (reg == REG_NOMATCH) {
        return HTTP_ERROR;
    }
    
    resource->data   = con->request.url.data + matches[1].rm_so;
    resource->length = matches[1].rm_eo - matches[1].rm_so;
    
    if (matches[2].rm_so >= 0) {
        resource_id->data   = con->request.url.data + matches[2].rm_so + 1;
        resource_id->length = matches[2].rm_eo - (matches[2].rm_so + 1);
    } else {
        resource_id->data   = NULL;
        resource_id->length = 0;
    }
        
    return HTTP_OK;
}

http_str_s select_query (http_str_s* resource) {
    
    http_str_s s;
    s.length = 0;
    s.data   = NULL;
    
    const char* template = "SELECT * FROM %.*s"; // TODO: see offeset to select, is necessary, if i has alot of items i dont wuant make process very slow
    s.length = asprintf(&s.data, template, resource->length, resource->data);
    
    return s;
    
}

http_str_s select_item_query (http_str_s* resource, http_str_s* resource_id) {
    
    http_str_s s;
    s.length = 0;
    s.data   = NULL;
    
    const char* template = "SELECT * FROM %.*s WHERE id = %.*s"; // TODO: see offeset to select, is necessary, if i has alot of items i dont wuant make process very slow
    s.length = asprintf(&s.data, template, resource->length, resource->data, resource_id->length, resource_id->data);
    
    return s;
    
}

http_str_s delete_item_query (http_str_s* resource, http_str_s* resource_id) {
    
    http_str_s s;
    s.length = 0;
    s.data   = NULL;
    
    const char* template = "DELETE FROM %.*s WHERE id = %.*s RETURNING *";
    s.length = asprintf(&s.data, template, resource->length, resource->data, resource_id->length, resource_id->data);

    return s;
    
}

