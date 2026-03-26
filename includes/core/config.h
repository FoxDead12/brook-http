#ifndef _BROOK_CONFIG_H
#define  _BROOK_CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include <cjson/cJSON.h>
#include <string.h>
#include "beanstalkclient.h"
#include "hiredis/async.h"
#include <time.h>

#define BROOK_OK    0
#define BROOK_ERROR -1
#define BROOK_DONE  -2

#define POOL_INDEX_BEANSTALKD 1
#define POOL_INDEX_REDIS 2


// ... internal includes ...
#include "types.h"
#include "socket.h"
#include "connection.h"
#include "gatekeeper.h"
#include "logger.h"

// ... server settings ...
extern int MAX_FD;       // ... i don't now if will be possible has 1024 connections, depende of systems settings
extern int CURRENT_FD;
extern bsc* bean_client;
extern redisAsyncContext* redis_client;
extern char brook_process_id[32];

extern struct pollfd* _fds;
extern brook_connection_t** _connections;

#endif
