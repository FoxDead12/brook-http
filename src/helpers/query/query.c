//
//  query.c
//  http-c-broker
//
//  Created by David Xavier on 13/07/2025.
//

#include "query.h"

int generate_query_from_request (http_connection_struct* con) {
    
    
    printf("METHOD: %.*s\n", con->request.method.length, con->request.method.data);
    printf("URL: %.*s\n",    con->request.url.length, con->request.url.data);
    
    printf("CONTENT_TYPE: %.*s\n",    con->request.headers.content_type.length, con->request.headers.content_type.data);
    
    
    return HTTP_OK;
}
