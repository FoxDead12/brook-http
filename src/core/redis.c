#include "core/redis.h"

redisAsyncContext* redis_client = NULL;

int
brook_redis_connect ( brook_conf_t* config ) {
  redis_client = redisAsyncConnect("127.0.0.1", 6380);

  if ( redis_client == NULL || redis_client->err ) {
    printf("Can't connect connect to redis: %s\n", redis_client->err);
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
    fprintf(stderr, "Erro: Redis desconectado inesperadamente: %s\n", c->errstr);
  } else {
    // A desconexão foi solicitada via redisAsyncDisconnect
    printf("Redis desconectado manualmente.\n");
  }
  exit(BROOK_ERROR);
}

void
brook_redis_on_connect (const redisAsyncContext *c, int status) {
  if ( status == -1 ) {
    perror("Can't connect connect to redis");
    exit(BROOK_ERROR);
  }
  // ... each process will has individual channel
  redisAsyncCommand(redis_client, brook_redis_on_message, NULL, "SUBSCRIBE %s", _process_brook_id);
  // ... redis add event of write ...
  _fds[POOL_INDEX_REDIS].events |= POLLOUT;
}

void
brook_redis_on_message ( redisAsyncContext* redis_con, void* message, void* data ) {

  redisReply *reply = message;
  // brook_conf_t* config = data;

  if ( !reply ) return;

  printf("channel: %s\n", reply->element[1]->str);
  printf("message: %s\n", reply->element[2]->str);

}
