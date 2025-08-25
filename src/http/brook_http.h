//
//  brook_http.h
//  http-c-broker
//
//  Created by David Xavier on 01/08/2025.
//

#ifndef brook_http_h
#define brook_http_h

#include "brook_config.h"
#include "brook_http_parse.h"
#include "brook_json_api.h"

typedef struct brook_http_s brook_http_t;
typedef struct brook_http_header_s brook_http_header_t;
typedef enum   brook_http_status_s brook_http_status_t;
typedef enum   brook_http_type_s   brook_http_type_t;
typedef enum   brook_http_method_s brook_http_method_t;

typedef struct brook_connection_s brook_connection_t;
typedef struct brook_gatekeeper_s brook_gatekeeper_t;

enum brook_http_method_s {
	GET,
	POST,
	PATCH,
	DELETE
};

enum brook_http_status_s {
	// ... action related to 'READING_SOCKET_MESSAGE' ...
	READ_HEADER,
	READ_BODY,

	// ... action relates to 'READING_REDIS_MESSAGE' ...
	VALIDATE_USER_RESPONSE,
	JOB_RESPONSE,

	// ... action relates to 'READING_PSQL_MESSAGE' ...
	JSON_API_RESPONSE,

	// ... action relates to 'WRITING_SOCKET_MESSAGE' ...
	WRITING_RESPONSE,

	// ... action relates to 'WRITING_REDIS_MESSAGE' ...
	VALIDATE_USER,

	// ... action relates to 'WRITING_BEANSTALK_MESSAGE' ...
	JOB_SEND,

	// ... action relates to 'WRITING_PSQL_MESSAGE' ...
	JSON_API_QUERY

};

enum brook_http_type_s {
	JSON_API,
	JOB
};

struct brook_http_header_s {
    brook_str_t     host;
    brook_str_t     connection;
    brook_str_t     content_type;
    int             content_length;
};

struct brook_http_s {
	brook_connection_t* connection;
	brook_http_status_t state;
	brook_http_type_t	type;

	brook_http_method_t method;
	brook_str_t         url;
	brook_str_t         params;
	brook_http_header_t header;

    brook_gatekeeper_t* gatekeeper_route;

	brook_buffer_t* _h;
	brook_chain_t* _b;
	brook_chain_t* _bp;
	
    //brook_json_api_t*   json_api;

	//brook_buffer_t*		buff_body;      // only pointer to buff of connection
	//brook_buffer_t*		buff_header;    // only pointer to buff of connection
};

#endif /* brook_http_h */
