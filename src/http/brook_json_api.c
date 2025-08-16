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
	
	printf("NEED VALIDATE IF RESOURCE EXIST AND PARSING THE DATA FROM OBJECT\n");
	
	// get resource in json object
	
    return BROOK_OK;
}

int
brook_json_api_setup_body (brook_http_t* request) {
	printf("CONVERT BUFFER TO JSON AND VALIDATE IF CONTAIN RIGHT FORMAT TO JSON API\n");
	return BROOK_OK;
}
