#ifndef _BROOK_GATEKEEPER_H
#define _BROOK_GATEKEEPER_H

#include "config.h"

typedef struct brook_gatekeeper_node_s {
  char* segment;    // "/first-example/:id" -> this route has two segments separate bar

  struct brook_gatekeeper_node_s* child;
  struct brook_gatekeeper_node_s* sibling;

  // ... jobs options ...
  char* tube;

} brook_gatekeeper_node_t;

int brook_gatekeeper_load(brook_conf_t* config);
brook_gatekeeper_node_t* brook_gatekeeper_create_node(const char* segment);
int brook_gatekeeper_insert_route(brook_gatekeeper_node_t* root, const char* path, const char* tube);

#endif
