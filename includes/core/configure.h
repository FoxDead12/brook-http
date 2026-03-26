#ifndef _BROOK_CONFIGURE_H
#define  _BROOK_CONFIGURE_H

#include "core/config.h"
#include <ctype.h>
int brook_load_configuration(brook_conf_t* config, const char* path);
void clean_value(char *dest, const char *src, int max_len);

#endif
