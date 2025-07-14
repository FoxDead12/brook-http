//
//  query.h
//  http-c-broker
//
//  Created by David Xavier on 13/07/2025.
//

#ifndef query_h
#define query_h

#include <stdio.h>
#include "../types/types.h"

int generate_query_from_request (http_connection_struct* con);

#endif /* query_h */
