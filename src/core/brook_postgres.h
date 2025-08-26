//
//  brook_postgres.h
//  http-c-broker
//
//  Created by David Xavier on 26/08/2025.
//

#ifndef brook_postgres_h
#define brook_postgres_h

#include "brook_config.h"


struct {
    PGconn** conns;
} brook_postgres_s;

#endif /* brook_postgres_h */
