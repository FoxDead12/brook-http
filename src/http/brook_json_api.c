//
//  brook_json_api.c
//  http-c-broker
//
//  Created by David Xavier on 08/08/2025.
//

#include "brook_json_api.h"
#include "brook_http.h"
#include "../core/brook_core.h"

const char* QUERY_GET    = "SELECT %s FROM %s WHERE %s ORDER BY %s LIMIT %s OFFSET %s";
const char* QUERY_DELETE = "DELETE FROM %s WHERE id = %s";
const char* QUERY_INSERT = "INSERT INTO %s VALUES %s";
const char* QUERY_UPDATE = "UPDATE %s SET %s WHERE %s";

int
brook_json_api_write_query (brook_http_t* request, PGconn* db) {
	
	if (request->json_api == NULL) {
		
        // ... need setup json api ...
		if (brook_json_api_setup(request) == BROOK_ERROR) {
			return BROOK_ERROR;
		}
            
	}
	
	// ... write to psql ...
    
	return BROOK_OK ;
}

int
brook_json_api_read_query (brook_http_t* request, PGconn* db) {
	return BROOK_OK;
}

int
brook_json_api_setup (brook_http_t* request) {
	
	brook_config_t* s_conf = request->connection->conf;
	
	// ... check gatekeeper contain resource data ...
	if (request->gatekeeper_route->resource == NULL) {
		return BROOK_ERROR;
	}
	
	// ... get resource config from gatekeeper settings ...
	json_object* server_resource = json_object_object_get(s_conf->resources, request->gatekeeper_route->resource);
	if (server_resource == NULL) {
		return BROOK_ERROR;
	}
	
	// ... create struct in request ...
	request->json_api 			  = malloc(sizeof(brook_json_api_t));
	request->json_api->request    = request;
	request->json_api->s_resource = server_resource;
	request->json_api->result 	  = json_object_new_object();
	
	// ... if method dont contain body out here ...
	if (request->method != POST && request->method != PATCH) {
		return BROOK_OK;
	}
	
	// ... check if exist body in request ...
	if (request->_b == NULL) {
		return BROOK_ERROR;
	}
	
    brook_json_api_query_t* query_s = malloc(sizeof(brook_json_api_query_t));
    query_s->table = json_get_str("table", server_resource, (brook_str_t) brook_string(""));
    
    // ... now the ideia is create all query templates ...
    if (request->method == GET) {
        
        
        
    } else if (request->method == DELETE) {
        
        
    } else if (request->method == POST) {
        
        
    } else if (request->method == PATCH) {
        
        
    } else return BROOK_ERROR;
    
    
    
    
    
    
    
	// ... validate body struct ...
	
    /*
    // ... get id ...
    request->json_api->id.data = NULL;
    request->json_api->id.len = 0;
    
	if (request->method == GET || request->method == DELETE || request->method == PATCH) {
		u_char* pointer = request->url.data + request->url.len - 1;
        while (pointer >= request->url.data && *pointer != '/') {
            pointer--;
        }
        
        if (pointer != request->url.data) {
            // ... exist id ...
            request->json_api->id.data = pointer + 1;
            request->json_api->id.len = (request->url.data + request->url.len) - request->json_api->id.data;
        }
		
		if (request->method != GET && request->json_api->id.data == NULL) {
			return BROOK_ERROR;
		}
    }
    
    // ... get params of url ...
    */
    return BROOK_OK;
}

int
brook_json_api_resource_get (brook_http_t* request) {
	return BROOK_OK;
}

int
brook_json_api_resource_delete (brook_http_t* request) {
    
    char* query = NULL;
	return BROOK_OK;
}

int
brook_json_api_resource_post (brook_http_t* request) {
	return BROOK_OK;
}

int
brook_json_api_resource_patch (brook_http_t* request) {
	return BROOK_OK;
}













int
brook_json_api_free (brook_http_t* request) {
	/*
	json_object_put(request->json_api->body);
	free(request->json_api);
	 */
    return BROOK_OK;
}

int
brook_json_api_setup_body (brook_http_t* request) {
    /*
    // ... transform body in json ...
    request->json_api->body = json_tokener_parse((char*) request->buff_body->start);
    
    // ... check if body contain god format of json api
    json_object* data = json_object_object_get(request->json_api->body, "data");
    if (data == NULL) return BROOK_ERROR;
    
    // ... type in body need bee equal to resource name in json ...
    json_object* type = json_object_object_get(data, "type");
    if (type == NULL) return BROOK_ERROR;
    if (brook_strncmp(request->gatekeeper_route->resource, json_object_get_string(type), json_object_get_string_len(type)) != 0) return BROOK_ERROR;
    
    // ... if method is PATCH and id is null is invalid, is necessary indicate id ...
    json_object* id = json_object_object_get(data, "id");
    if (request->method == PATCH && id == NULL) return BROOK_ERROR;
    
    // ... check if contain attributes ...
    json_object* attributes = json_object_object_get(data, "attributes");
    if (attributes == NULL) return BROOK_ERROR;
    
    // ... set data to store in object ...
    request->json_api->data = data;
    
    // ... set type ...
    request->json_api->type.data = (u_char*) json_object_get_string(type);
    request->json_api->type.len  = json_object_get_string_len(type);
    
    // ... set id ...
    if (id == NULL) {
        request->json_api->type.data = NULL;
        request->json_api->type.len  = 0;
    } else {
        request->json_api->type.data = (u_char*) json_object_get_string(id);
        request->json_api->type.len  = json_object_get_string_len(id);
    }
    
    request->json_api->attributes = attributes;
    */
	return BROOK_OK;
}
