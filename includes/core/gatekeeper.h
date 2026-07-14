#ifndef _BROOK_GATEKEEPER_H
#define _BROOK_GATEKEEPER_H

#include "config.h"

int brook_gatekeeper_load(brook_conf_t* config);
void brook_gatekeeper_free(brook_gatekeeper_node_t* root);
brook_gatekeeper_node_t* brook_gatekeeper_create_node(const char* path, const char* tube, const char* product_key, uint32_t method_mask, int role_mask, char* job_options);
brook_gatekeeper_node_t* brook_gatekeeper_insert_route(brook_gatekeeper_node_t* root, const char* path, const char* tube, const char* product_key, uint32_t method_mask, int role_mask, char* job_options);
brook_gatekeeper_node_t* brook_gatekeeper_match_route(brook_gatekeeper_node_t *root, brook_str_t url, uint32_t method);
int brook_gatekeeper_validate_session(brook_connection_t* con);
#endif
