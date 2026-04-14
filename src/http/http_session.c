#include "http/http_session.h"


/**
 * Method to start the logic to validate the user session
 */
int
brook_get_client_session ( brook_connection_t* con ) {

  // ... need parse token of user ...
  brook_str_t token = brook_parse_token_from_cookies(con);
  if ( token.len < 1 ) {
    brook_connection_reply(con, 401, (brook_str_t) brook_str("Unauthorized"), (brook_str_t) brook_str("Token not found"));
    return BROOK_ERROR;
  }

  brook_log(con->_config, LOG_DEBUG, "token encontrado: %.*s\n", token.len, token.data);

  return BROOK_OK;
}

brook_str_t
brook_parse_token_from_cookies ( brook_connection_t* con ) {
  const char* key = "token";
  int index_max = strlen(key) - 1;
  int index = 0;
  brook_str_t token = {0, NULL};

  // ... search in cookies the token key ...
  for ( int i = 0; i < con->_parser->cookies.len; i++ ) {
    // ... get char ...
    unsigned char c = con->_parser->cookies.data[i];
    if ( index >= 0 ) {
      // ... check if match the key ...
      if (i > 0 && index == 0 && con->_parser->cookies.data[i-1] != ' ' && con->_parser->cookies.data[i-1] != ';') {
        continue;
      }
      if ( c == key[index] ) {
        index++;
      } else if ( index > index_max && c == '=' ) {
        if ( i + 1 < con->_parser->cookies.len ) {
          token.data = &con->_parser->cookies.data[i + 1];
          token.len = 0;
        } else {
          // ... dont exist value ...
          break;
        }
        index = -1;   // to say we catch the key
      } else {
        index = 0;
      }
    } else if ( index == -1 ) {
      if (c == ';' || c == '\r' || c == '\n' || c == ' ' || c == '\t') {
        break;
      }
      token.len++;
    }
  }
  return token;
}
