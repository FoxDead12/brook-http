//
//  brook_json_api.h
//  http-c-broker
//
//  Created by David Xavier on 08/08/2025.
//

#ifndef brook_json_api_h
#define brook_json_api_h

#include "../core/brook_string.h"

typedef struct brook_json_api_s brook_json_api_t;
typedef struct brook_http_s brook_http_t;
typedef struct brook_json_api_data_s brook_json_api_data_t;

struct brook_json_api_data_s {
	brook_str_t type;
	brook_str_t id;
	brook_array_t* attributes;
	brook_array_t* relationships;
};

struct brook_json_api_s {
	brook_array_t* included;
	brook_array_t* filters;
	brook_str_t order_by;
};

int brook_json_api_setup (brook_http_t* request);
int brook_json_api_setup_body (brook_http_t* request);

#endif /* brook_json_api_h */
