#ifndef _BROOK_REDIS_H
#define  _BROOK_REDIS_H

#include "core/config.h"
#include "hiredis/async.h"
#include <hiredis/sds.h>
#include "http/http_response.h"


int brook_redis_connect(brook_conf_t* config);
int brook_redis_write();
void brook_redis_on_disconnect(const redisAsyncContext *c, int status);
void brook_redis_on_connect(const redisAsyncContext *c, int status);
void brook_redis_on_message(redisAsyncContext* redis_con, void* r, void* privdata);

#endif
