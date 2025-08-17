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
    
    request->json_api = malloc(sizeof(brook_json_api_t));

    
    
    return BROOK_OK;
}

int
brook_json_api_free (brook_http_t* request) {
    free(request->json_api);
    return BROOK_OK;
}

int
brook_json_api_setup_body (brook_http_t* request) {
	printf("CONVERT BUFFER TO JSON AND VALIDATE IF CONTAIN RIGHT FORMAT TO JSON API\n");
	return BROOK_OK;
}

int
brook_json_api_find_resource (brook_http_t* request) {
    
    /*char key[1024] = {0};
    snprintf(key, 1024, "%.*s", (int) request->url.len, request->url.data);
    
    json_object* object = NULL;
    if (json_object_object_get_ex(request->connection->conf->resources, key, &object)) {
        
    }*/
    
    return BROOK_OK;
}
