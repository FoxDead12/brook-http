#ifndef _BROOK_GATEKEEPER_H
#define _BROOK_GATEKEEPER_H

#include "config.h"

int brook_gatekeeper_load(brook_conf_t* config);
brook_gatekeeper_node_t* brook_gatekeeper_create_node ( const char* path, const char* tube, uint32_t method_mask );
brook_gatekeeper_node_t* brook_gatekeeper_insert_route(brook_gatekeeper_node_t* root, const char* path, const char* tube, uint32_t method_mask);
brook_gatekeeper_node_t* brook_gatekeeper_match_route(brook_gatekeeper_node_t *root, brook_str_t url, uint32_t method);

#endif
