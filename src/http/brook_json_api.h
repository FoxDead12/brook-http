//
//  brook_json_api.h
//  http-c-broker
//
//  Created by David Xavier on 08/08/2025.
//

#ifndef brook_json_api_h
#define brook_json_api_h

#include "../core/brook_string.h"

typedef struct brook_http_s 		 brook_http_t;
typedef struct brook_json_api_s		 brook_json_api_t;


typedef struct brook_json_api_data_s brook_json_api_data_t;
struct brook_json_api_data_s {
	json_object* type;
	json_object* id;
	json_object* attributes;
	json_object* relationships;
};





struct brook_json_api_s {
	brook_http_t* request;
	json_object* s_resource;
	
	json_object* result; // ... result of json api, will be response of request ...

};

/*
struct brook_json_api_data_s {
	brook_str_t type;
	brook_str_t id;
	brook_array_t* attributes;
};

struct brook_json_api_s {
    json_object*   resource; // ... pointer of resource in server config ...
};
*/

int brook_json_api_write_query(brook_http_t* request, PGconn* db);
int brook_json_api_read_query(brook_http_t* request, PGconn* db);
int brook_json_api_setup(brook_http_t* request);

int brook_json_api_resource_get(brook_http_t* request);


#endif /* brook_json_api_h */
