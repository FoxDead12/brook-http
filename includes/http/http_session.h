#ifndef _BROOK_HTTP_SESSION_H
#define  _BROOK_HTTP_SESSION_H

#include "core/config.h"


enum token_parse {
  s_token_user_id,
  s_token_base64,
  s_token_base64_end
};


int brook_session_get_client_session(brook_connection_t* con);
int brook_session_validate_token_formater(brook_connection_t* con, brook_str_t token);
brook_str_t brook_session_parse_token_from_cookies(brook_connection_t* con);


#endif
