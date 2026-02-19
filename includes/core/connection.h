#ifndef _BROOK_CONNECTION_H
#define _BROOK_CONNECTION_H

#include "config.h"

int brook_handle_connection(brook_conf_t* config);
int brook_add_connection(brook_connection_t* con);
int brook_connection_read(brook_connection_t* con);

#endif
