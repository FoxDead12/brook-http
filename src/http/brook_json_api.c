//
//  brook_json_api.c
//  http-c-broker
//
//  Created by David Xavier on 08/08/2025.
//

#include "brook_json_api.h"
#include "brook_http.h"
#include "../core/brook_core.h"

int
brook_json_api_setup (brook_http_t* request) {
    
    // ... check if gatekeeper resource contain resource ...
    if (request->gatekeeper_route->resource == NULL) {
        return BROOK_ERROR;
    }
    
    // ... get resource ...
    json_object* resource = json_object_object_get(request->connection->conf->resources, request->gatekeeper_route->resource);
    if (resource == NULL) {
        return BROOK_ERROR;
    }
    
    // ... create object of json api ...
    request->json_api = malloc(sizeof(brook_json_api_t));
    request->json_api->resource = resource;
    request->json_api->body = NULL;
    
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
    
    printf("id: %.*s\n", request->json_api->id.len, request->json_api->id.data);

    
    return BROOK_OK;
}

int
brook_json_api_free (brook_http_t* request) {
	json_object_put(request->json_api->body);
	free(request->json_api);
    return BROOK_OK;
}

int
brook_json_api_setup_body (brook_http_t* request) {
    
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
    
	return BROOK_OK;
}
