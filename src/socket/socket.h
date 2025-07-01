//
//  socket.h
//  http-c-broker
//
//  Created by David Xavier on 24/06/2025.
//

#ifndef socket_h
#define socket_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "../helpers/types/types.h"
#include "../helpers/json/json.h"
#include "../helpers/request/request.h"

int socket_init (int port);
http_request_struct* socket_new_connection (http_main_struct *conf);
int socket_disconect_connection (http_request_struct *client);

int socket_new_message (http_request_struct* client);
int socket_message_http_header (http_request_struct* client);

#endif /* socket_h */
