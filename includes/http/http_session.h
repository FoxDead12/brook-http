#ifndef _BROOK_HTTP_SESSION_H
#define  _BROOK_HTTP_SESSION_H

#include "core/config.h"

int brook_get_client_session(brook_connection_t* con);
brook_str_t brook_parse_token_from_cookies(brook_connection_t* con);


#endif
