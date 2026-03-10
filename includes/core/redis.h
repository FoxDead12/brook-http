#ifndef _BROOK_REDIS_H
#define  _BROOK_REDIS_H

#include "core/config.h"
#include "hiredis/async.h"

int brook_redis_connect(brook_conf_t* config);
void brook_redis_on_message(redisAsyncContext* redis_con, void* r, void* privdata);

#endif
