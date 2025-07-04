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

int socket_init (int port);
int socket_connection (int socket, struct sockaddr_in* client_addr);

/*http_request_struct* socket_new_connection (http_main_struct *conf);
int                  socket_disconect_connection (http_request_struct *client);

int                  socket_new_message (http_request_struct* client);
int                  socket_message_http_header (http_request_struct* client);*/

#endif /* socket_h */
