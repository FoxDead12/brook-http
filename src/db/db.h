//
//  db.h
//  http-c-broker
//
//  Created by David Xavier on 05/07/2025.
//

#ifndef db_h
#define db_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libpq-fe.h>
#include <string.h>
#include <errno.h>
#include "../helpers/types/types.h"

int init_db_connections (http_db_pool_struct* pool, http_str_s conn_str);

#endif /* db_h */
