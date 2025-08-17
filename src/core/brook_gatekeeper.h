//
//  brook_gatekeeper.h
//  http-c-broker
//
//  Created by David Xavier on 17/08/2025.
//

#ifndef brook_gatekeeper_h
#define brook_gatekeeper_h

#include "brook_config.h"

typedef struct brook_connection_s brook_connection_t;
typedef struct brook_gatekeeper_s brook_gatekeeper_t;

struct brook_gatekeeper_s {
    brook_array_t*  methods;
    regex_t         route;
    char*           resource;
};

int brook_gatekeeper_build(brook_config_t* conf, json_object* json);
int brook_gatekeeper_validate(brook_connection_t* connection);

#endif /* brook_gatekeeper_h */
