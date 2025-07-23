//
//  query.h
//  http-c-broker
//
//  Created by David Xavier on 13/07/2025.
//

#ifndef query_h
#define query_h

#include <stdio.h>
#include <stdarg.h>
#include "../types/types.h"

int         generate_query_from_request (http_connection_struct* con, PGconn* db);
int         parse_resource_data         (http_connection_struct* con, http_str_s* resource, http_str_s* resource_id);
http_str_s  select_query                (http_str_s* resource);
http_str_s select_item_query            (http_str_s* resource, http_str_s* resource_id);
http_str_s delete_item_query            (http_str_s* resource, http_str_s* resource_id);
http_str_s insert_item_query            (http_str_s* resource, json_object* attributes);
http_str_s update_item_query (http_str_s* resource, http_str_s* resource_id, json_object* attributes);
#endif /* query_h */
