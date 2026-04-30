#ifndef _BROOK_REDIS_H
#define  _BROOK_REDIS_H

#include "core/config.h"
#include "hiredis/async.h"
#include <hiredis/sds.h>
#include "http/http_response.h"


int brook_redis_connect(brook_conf_t* config, int subescriber);
int brook_redis_retry_connect(brook_conf_t* config, int subescriber);
int brook_redis_create_client(brook_conf_t* config, redisAsyncContext** client, redisConnectCallback *connected_callback);
void brook_redis_on_connect(const redisAsyncContext *c, int status);
void brook_redis_sub_on_connect(const redisAsyncContext *c, int status);
void brook_redis_on_disconnect(const redisAsyncContext *c, int status);
int brook_redis_write(redisAsyncContext* client, int POOL_INDEX);
void brook_redis_on_subescribe_message(redisAsyncContext* redis_con, void* message, void* _);
int brook_redis_get_session(brook_connection_t* con, brook_str_t token);
void brook_redis_on_get_session(redisAsyncContext *c, void *repl, void *privdata);

#endif
