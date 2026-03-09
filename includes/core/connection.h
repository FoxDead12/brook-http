#ifndef _BROOK_CONNECTION_H
#define _BROOK_CONNECTION_H

#include "config.h"

int brook_handle_connection(brook_conf_t* config);
int brook_connection_read(brook_connection_t* con);
int brook_add_connection(brook_connection_t* con);
int brook_connection_reply(brook_connection_t* con, uint16_t code, brook_str_t message, brook_str_t detail);
int brook_connection_write(brook_connection_t* con);
int brook_destroy_connection(brook_connection_t* con);

#endif
