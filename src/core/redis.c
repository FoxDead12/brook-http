#include "core/redis.h"

redisAsyncContext* redis_client = NULL;

int
brook_redis_connect ( brook_conf_t* config ) {
  redis_client = redisAsyncConnect(config->redis.host, config->redis.port);

  if ( redis_client == NULL || redis_client->err ) {
    kill(getppid(), SIGTERM);
    brook_log(config, LOG_ERR, " Can't create redis client: %s\n", redis_client->err);
    return BROOK_ERROR;
  }

  redisAsyncSetConnectCallback(redis_client, brook_redis_on_connect);
  redisAsyncSetDisconnectCallback(redis_client, brook_redis_on_disconnect);
  return BROOK_OK;
}

int
brook_redis_write () {
  redisAsyncHandleWrite(redis_client);
  // ... check if can clean event ...
  if (redis_client->c.obuf == NULL || sdslen(redis_client->c.obuf) == 0) {
    _fds[POOL_INDEX_REDIS].events &= ~POLLOUT;
  }
  return BROOK_OK;
}

void
brook_redis_on_disconnect (const redisAsyncContext *c, int status) {
  if (status != REDIS_OK) {
    // A desconexão foi causada por um erro
    brook_log(NULL, LOG_ERR, " Redis connection lost unexpectedly. Reason: %s\n", c->errstr);
  } else {
    // A desconexão foi solicitada via redisAsyncDisconnect
    //printf("Redis desconectado manualmente.\n");
  }
  sleep(5);
  exit(BROOK_ERROR);
}

void
brook_redis_on_connect (const redisAsyncContext *c, int status) {
  if ( status == -1 ) {
    perror("Can't connect connect to redis");
    brook_log(NULL, LOG_ERR, " Can't connect connect to redis: %s\n", c->errstr);
    kill(getppid(), SIGTERM);
    exit(BROOK_ERROR);
  }
  // ... each process will has individual channel
  redisAsyncCommand(redis_client, brook_redis_on_message, NULL, "SUBSCRIBE %s", brook_process_id);
  // ... redis add event of write ...
  _fds[POOL_INDEX_REDIS].events |= POLLOUT;

  brook_log(NULL, LOG_INFO, " Process connected to redis ...\n");

}

void
brook_redis_on_message ( redisAsyncContext* redis_con, void* message, void* _ ) {

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
