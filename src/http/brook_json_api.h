//
//  brook_json_api.h
//  http-c-broker
//
//  Created by David Xavier on 08/08/2025.
//

#ifndef brook_json_api_h
#define brook_json_api_h

typedef struct brook_json_api_s brook_json_api_t;
typedef struct brook_connection_s brook_connection_t;

struct brook_json_api_s {
    brook_connection_t* connection;
};

int brook_json_api_validate_request(brook_connection_t* connection);

#endif /* brook_json_api_h */
