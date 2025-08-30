//
//  brook_json_api.h
//  http-c-broker
//
//  Created by David Xavier on 08/08/2025.
//

#ifndef brook_json_api_h
#define brook_json_api_h

#include "../core/brook_string.h"

typedef struct brook_http_s 		  brook_http_t;
typedef struct brook_json_api_s		  brook_json_api_t;
typedef struct brook_json_api_data_s  brook_json_api_data_t;
typedef struct brook_json_api_query_s brook_json_api_query_t;

struct brook_json_api_data_s {
	json_object* type;
	json_object* id;
	json_object* attributes;
	json_object* relationships;
};

struct brook_json_api_s {
	brook_http_t* request;
	json_object* s_resource; // ... resource object of server config ...
	json_object* result; // ... result of json api, will be response of request ...

};

struct brook_json_api_query_s {
    char*          query_template;
    char*          query;
    brook_str_t    table;
    brook_array_t* filter;
    brook_str_t    order;
    brook_str_t    limit;
    brook_str_t    offset;
    json_object*   attributes;
};

int brook_json_api_write_query(brook_http_t* request, PGconn* db);
int brook_json_api_read_query(brook_http_t* request, PGconn* db);
int brook_json_api_setup(brook_http_t* request);

int brook_json_api_resource_get(brook_http_t* request);


#endif /* brook_json_api_h */
