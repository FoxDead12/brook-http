#include "http/http_session.h"
#include "http/http_request.h"
#include "core/redis.h"

/**
 * Method to start the logic to validate the user session
 */
int
brook_session_get_client_session ( brook_connection_t* con ) {

  // ... need parse token of user ...
  brook_str_t token = brook_session_parse_token_from_cookies(con);
  if ( token.len < 1 ) {
    brook_connection_reply(con, 401, (brook_str_t) brook_str("Unauthorized"), (brook_str_t) brook_str("Token not found"));
    return BROOK_ERROR;
  }

  brook_log(con->_config, LOG_DEBUG, "token encontrado: %.*s\n", token.len, token.data);

  // ... check token formater ...
  if ( brook_session_validate_token_formater(con, token) == BROOK_ERROR ) {
    brook_connection_reply(con, 401, (brook_str_t) brook_str("Unauthorized"), (brook_str_t) brook_str("Token invalid format"));
    return BROOK_ERROR;
  }

  // ... send message to redis get session ...
  if ( brook_redis_get_session(con, token) == BROOK_ERROR ) {
    brook_connection_reply(con, 401, (brook_str_t) brook_str("Unauthorized"), (brook_str_t) brook_str("Cant send redis message"));
    return BROOK_ERROR;
  }

  // ... store token in session ...
  con->session.token = token;

  return BROOK_OK;
}

int
brook_session_validate_token_formater ( brook_connection_t* con, brook_str_t token ) {

  // const regex_cookie = /^([A-Za-z0-9]+-[A-Za-z0-9+\/]{86}==)$/;
  int state = s_token_user_id;
  int index = 0;

  for ( int i = 0; i < token.len; i++ ) {

    // ... get char ...
    unsigned char c = token.data[i];

    // ... separate logic by state ...
    switch (state) {

      // ... validate user id ir write in token ...
      case s_token_user_id:
      {
        // ... validate if is a number ...
        if ( !IS_NUM(c) ) {
          if ( i > 0 && c == '-' ) { // ... separator of token ...
            state = s_token_base64;
            index = 0;
          } else {
            return BROOK_ERROR;
          }
        }
        break;
      }

      // ... validate base 64 data ...
      case s_token_base64:
      {
        // ... check if is a valid byte ...
        if ( !IS_BASE64(c) ) {
          printf("invalid base 64 byte: %c\n", c);
          return BROOK_ERROR;
        }
        index++;
        if ( index == 86 ) {
          state = s_token_base64_end;
          index = 0;
        }
        break;
      }

      case s_token_base64_end:
      {
        if (c == '=') {
          index++;
          if (index > 2) {
            printf("to biig data in end\n");
            return BROOK_ERROR;
          }
        } else {
          printf("end of token is invalid\n");
          return BROOK_ERROR;
        }
        break;
      }
    }
  }

  if ( state != s_token_base64_end && index != 2 ) {
    printf("token não esta bem\n");
    return BROOK_ERROR;
  }

  return BROOK_OK;
}

/**
 * Method used to parse access token from cookies header value
 * this function is possible make general to catch keys values in a string
 */
brook_str_t
brook_session_parse_token_from_cookies ( brook_connection_t* con ) {
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
