//
//  brook_gatekeeper.c
//  http-c-broker
//
//  Created by David Xavier on 17/08/2025.
//

#include "brook_gatekeeper.h"
#include "brook_core.h"
#include "../http/brook_http.h"

int
brook_gatekeeper_build (brook_config_t* conf, json_object* json) {
    
    size_t length = json_object_array_length(json);
    conf->gatekeeper = brook_create_array(length, sizeof(brook_gatekeeper_t));
    
    for (int i = 0; i < length; i++) {
        
        // ... create struct of gatekeeper
        conf->gatekeeper->data[i] = malloc(sizeof(brook_gatekeeper_t));
        
        // ... get object of gatekeeper json
        json_object *j = json_object_array_get_idx(json, i);
        brook_gatekeeper_t* r = conf->gatekeeper->data[i];
        
        // ... create route in regex to validate
        brook_str_t route = json_get_str("route", j, (brook_str_t) brook_string(""));
        if (regcomp(&r->route, (char*) route.data, REG_EXTENDED)) {
            perror((char*)route.data);
            return BROOK_ERROR;
        }
        
        // ... create methods array
        brook_array_t* methods = json_get_array("method", j);
        r->methods = brook_create_array(methods->size, sizeof(int));
        
        for (int c = 0; c < methods->size; c++) {
            if (brook_strncmp(methods->data[c], "GET", 3) == 0) {
                r->methods->data[c] = (void*) GET;
            }
            else if (brook_strncmp(methods->data[c], "POST", 4) == 0) {
                r->methods->data[c] = (void*) POST;
            }
            else if (brook_strncmp(methods->data[c], "PATCH", 5) == 0) {
                r->methods->data[c] = (void*) PATCH;
            }
            else if (brook_strncmp(methods->data[c], "DELETE", 6) == 0) {
                r->methods->data[c] = (void*) DELETE;
            }
        }
        free(methods);

        
        brook_str_t resource_name = json_get_str("resource", j, (brook_str_t) brook_string(""));
        if (resource_name.len > 0) {
            r->resource = calloc(1, resource_name.len);
            snprintf(r->resource, resource_name.len + 1, "%.*s", (int) resource_name.len, (char*) resource_name.data);
        } else {
            r->resource = NULL;
        }
        
    }
    
    return BROOK_OK;
}

int
brook_gatekeeper_validate (brook_connection_t* connection) {

    size_t len = connection->conf->gatekeeper->size;
    brook_http_t* request = connection->http;
    
    for (int i = 0; i < len; i++) {
        
        brook_gatekeeper_t* r = connection->conf->gatekeeper->data[i];
        
        if (regexec(&r->route, (char*) request->url.data, 0, NULL, 0) == REG_NOMATCH) {
            continue;
        }
        
        size_t len_m = r->methods->size;
        bool find_method = false;
        
        for (int c = 0; c < len_m; c++) {
            if (r->methods->data[c] == (void*) request->method) {
                find_method = true;
            }
        }
        
        if (find_method == false) return BROOK_ERROR;
        
        connection->http->gatekeeper_route = r;
        return BROOK_OK;
        
    }
    
    return BROOK_ERROR;
}
