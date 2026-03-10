#include "core/redis.h"

redisAsyncContext* redis_client = NULL;

int
brook_redis_connect ( brook_conf_t* config ) {
  redis_client = redisAsyncConnect("127.0.0.1", 6380);

  if ( redis_client == NULL || redis_client->err ) {
    printf("Can't connect connect to redis: %s\n", redis_client->err);
    return BROOK_ERROR;
  }

  redisAsyncCommand(redis_client, brook_redis_on_message, config, "SUBSCRIBE brook");

  return BROOK_OK;
}

void
brook_redis_on_message ( redisAsyncContext* redis_con, void* message, void* data ) {

  redisReply *reply = message;
  brook_conf_t* config = data;

  if ( !reply ) return;

  printf("channel: %s\n", reply->element[1]->str);
  printf("message: %s\n", reply->element[2]->str);

}
