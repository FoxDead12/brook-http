#include "core/redis.h"
#include "core/beanstalkd.h"

redisAsyncContext* redis_client = NULL;
redisAsyncContext* redis_client_sub = NULL;

int
brook_redis_connect ( brook_conf_t* config, int subescriber ) {
  int result = BROOK_OK;

  if (subescriber == 1) {
    // ... is subescriber connection ...
    result = brook_redis_create_client(config, &redis_client_sub, brook_redis_sub_on_connect);
  } else {
    // ... is normal connection ...
    result = brook_redis_create_client(config, &redis_client, brook_redis_on_connect);
  }

  return result;
}

/**
 * Method to create a client connection to a pointer
 */
int
brook_redis_create_client ( brook_conf_t* config, redisAsyncContext** client, redisConnectCallback *connected_callback ) {

  // ... create client ...
  *client = redisAsyncConnect(config->redis.host, config->redis.port);
  (*client)->data = config;

  // ... point to original connection pointer ...
  redisAsyncContext* c = *client;

  // ... validate struct is beed created ...
  if ( c == NULL || c->err ) {
    kill(getppid(), SIGTERM);
    brook_log(config, LOG_ERR, "Can't create redis client: %s\n", c->err);
    return BROOK_ERROR;
  }

  // ... set callbacks to clients
  redisAsyncSetConnectCallback(c, connected_callback);
  redisAsyncSetDisconnectCallback(c, brook_redis_on_disconnect);

  return BROOK_OK;
}

/**
 * Method used to connect server to redis
 */
void
brook_redis_on_connect (const redisAsyncContext *c, int status) {
  // ... get config from redis connection ...
  brook_conf_t *config = (brook_conf_t *)c->data;

  // ... check status result ...
  if ( status == -1 ) {
    perror("Can't connect connect to redis");
    brook_log(config, LOG_ERR, "Can't connect connect to redis: %s\n", c->errstr);
    kill(getppid(), SIGTERM);
    exit(BROOK_ERROR);
  }

  brook_log(config, LOG_INFO, "Process connected to redis ...\n");
}

/**
 * Method used to connect server to redis and create subescriber connection
 */
void
brook_redis_sub_on_connect (const redisAsyncContext *c, int status) {
  // ... get config from redis connection ...
  brook_conf_t *config = (brook_conf_t *)c->data;

  // ... check status result ...
  if ( status == -1 ) {
    perror("Can't connect connect to redis");
    brook_log(config, LOG_ERR, "Can't connect connect to redis: %s\n", c->errstr);
    kill(getppid(), SIGTERM);
    exit(BROOK_ERROR);
  }

  // ... each process will has individual channel
  redisAsyncCommand(redis_client_sub, brook_redis_on_subescribe_message, NULL, "SUBSCRIBE %s", brook_process_id);

  // ... redis add event of write ...
  _fds[POOL_INDEX_REDIS_SUBSCRIBER].events |= POLLOUT;

  brook_log(config, LOG_INFO, "Process connected to redis ...\n");
}

/**
 * Method called when redis client disconnect
 */
void
brook_redis_on_disconnect (const redisAsyncContext *c, int status) {
  // ... get config from redis connection ...
  brook_conf_t *config = (brook_conf_t *)c->data;

  if (status != REDIS_OK) {
    brook_log(config, LOG_ERR, "Redis connection lost unexpectedly. Reason: %s\n", c->errstr);
  } else {
    // A desconexão foi solicitada via redisAsyncDisconnect
    //printf("Redis desconectado manualmente.\n");
  }
  sleep(5);
  exit(BROOK_ERROR);
}

/**
 * Method called from poll() event to write redis socket messages
 */
int
brook_redis_write (redisAsyncContext* client, int POOL_INDEX) {
  // ... wirte data in socket ...
  redisAsyncHandleWrite(client);

  // ... check if can clean event ...
  if (client->c.obuf == NULL || sdslen(client->c.obuf) == 0) {
    _fds[POOL_INDEX].events &= ~POLLOUT;
  }

  return BROOK_OK;
}

/**
 * Method used to handle subescriber messages comming from redis
 */
void
brook_redis_on_subescribe_message ( redisAsyncContext* redis_con, void* message, void* _ ) {

  redisReply *reply = message;
  if ( !reply ) return;
  if ( !reply->element[2]->str ) return;

  // ... we assume this message is alway job response ...
  /*
    response payload:
    {
      job_id: 123
      headers: {}
      payload: {}
    }
  */

  cJSON* data = cJSON_Parse(reply->element[2]->str);
  if ( data == NULL ) {
    // ... ignore message, if json is invalid
    //printf("receive a invalid message from redis.\n");
    return;
  }

  // ... parse keys of main object ...
  brook_connection_t* con = NULL;
  cJSON* _j_job_id  = NULL;
  cJSON* _j_headers = NULL;
  cJSON* _j_payload = NULL;
  cJSON* _j_status = NULL;
  uint64_t job_id = 0;
  uint16_t status = 0;

  _j_job_id  = cJSON_GetObjectItemCaseSensitive(data, "job_id");
  _j_headers = cJSON_GetObjectItemCaseSensitive(data, "headers");
  _j_status = cJSON_GetObjectItemCaseSensitive(data, "status");
  _j_payload = cJSON_GetObjectItemCaseSensitive(data, "payload");

  if ( !cJSON_IsNumber(_j_job_id) ) {
    cJSON_Delete(data);
    return;
  }

  job_id = _j_job_id->valueint;
  status = _j_status->valueint;

  for ( int i = CURRENT_FD; i >= 0; i-- ) {
    if ( _connections[i] != NULL ) {
      if ( _connections[i]->job.id == job_id ) {
        con = _connections[i];
        break;
      }
    }
  }

  if ( con == NULL ) {
    cJSON_Delete(data);
    return;
  }

  brook_http_response_add_status(con, status);
  brook_http_response_add_header(con, (brook_str_t) brook_str("Content-Type: application/json"));
  brook_http_response_add_header(con, (brook_str_t) brook_str("Server: brook-http"));

  // ... add custom headers to response ...
  cJSON* _h_item = NULL;
  cJSON_ArrayForEach(_h_item, _j_headers) {
    const char* key = _h_item->string;
    if (!key) continue;

    if ( cJSON_IsArray(_h_item) ) {
      // ... value is array ...
      cJSON* _value = NULL;
      cJSON_ArrayForEach(_value, _h_item) {
        if (cJSON_IsString(_value) && _value->valuestring != NULL) {
          const char* value = _value->valuestring;
          brook_http_response_add_header_json(con, (brook_str_t) {strlen(key), key}, (brook_str_t) {strlen(value), value});
        }
      }
    } else if (cJSON_IsString(_h_item) && _h_item->valuestring != NULL) {
      // ... value is a string ...
      const char* value = _h_item->valuestring;
      brook_http_response_add_header_json(con, (brook_str_t) {strlen(key), key}, (brook_str_t) {strlen(value), value});
    }

  }

  // ... we have json ...
  brook_str_t body;
  body.data = cJSON_PrintUnformatted(_j_payload);
  body.len = strlen((const char*) body.data);

  brook_http_response_add_content_length(con, body.len);
  brook_http_response_add_body(con, body);

  free(body.data);
  cJSON_Delete(data);

  con->_pfd->events = POLLOUT;      // ... change events of poll socket
  return;
}

/**
 * Method used to get access token from redis
 */
int
brook_redis_get_session (brook_connection_t* con, brook_str_t token) {

  // ... submit command to redis ...
  int status = redisAsyncCommand(redis_client, brook_redis_on_get_session, con, "HGETALL user:token:%b", token.data, token.len);

  // ... check if error happend generating command ...
  if (status != REDIS_OK) {
    return BROOK_ERROR;
  }

  // ... redis add event of write ...
  _fds[POOL_INDEX_REDIS].events |= POLLOUT;

  return BROOK_OK;
}

void
brook_redis_on_get_session ( redisAsyncContext *c, void *repl, void *privdata ) {

  // ... sanity check if connection is off ...
  if ( !privdata ) {
    return;
  };

  // ... get config from redis connection ...
  brook_conf_t *config = (brook_conf_t *)c->data;

  // ... get connection from callback result ...
  redisReply *reply = repl;
  brook_connection_t *con = privdata;

  // ... check if connection was already responde ...
  if ( con->_reponse.status > 0 ) {
    return BROOK_DONE;
  }

  // ... validate if error append or empty response ...
  if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
    brook_connection_reply(con, 401, (brook_str_t) brook_str("Unauthorized"), (brook_str_t) brook_str("Getting session"));
    return;
  }

  // ... parse key of redis to internal struct ...
  if (reply->type == REDIS_REPLY_ARRAY) {
    for (size_t i = 0; i < reply->elements; i += 2) {

      // ... get key and value from record ...
      redisReply *key = reply->element[i];
      redisReply *val = reply->element[i+1];

      // ... sanity check, key and value must be strings ...
      if (key->type != REDIS_REPLY_STRING || val->type != REDIS_REPLY_STRING) {
        continue;
      }

      if (strncmp(key->str, "user_roles", key->len) == 0) {
        // ... get role mask from session ...
        con->session.role_mask = (uint32_t) strtoul(val->str, NULL, 16);

      } else if (strncmp(key->str, "user_id", key->len) == 0) {
        // ... get user id from session ...
        con->session.user_id = atoi(val->str);

      } else if (strncmp(key->str, "user_schema", key->len) == 0) {
        // ... get user id from session ...
        strncpy(con->session.schema, val->str, sizeof(con->session.schema) - 1);

      }
    }
  }

  // ... check if session is ok ...
  if ( con->session.role_mask == 0 || con->session.user_id == 0 ) {
    brook_log(config, LOG_WARN, "Invalid session comming from redis: %.*s\n", con->session.token.len, con->session.token.data);
    brook_connection_reply(con, 401, (brook_str_t) brook_str("Unauthorized"), (brook_str_t) brook_str("Invalid session"));
    return;
  }

  // ... validate gatekeeper route role mask ...
  if ( (con->_role->role_mask & con->session.role_mask) == con->_role->role_mask ) {
    // ... session is valid submit job ...
    brook_benstalkd_create_job(con);
  } else {
    brook_connection_reply(con, 403, (brook_str_t) brook_str("Forbidden"), (brook_str_t) brook_str("No permission"));
  }

  return;
}
