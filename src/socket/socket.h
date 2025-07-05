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

int     socket_init (int port);
int     socket_connection (int socket, struct sockaddr_in* client_addr);
size_t  socket_read (int socket, char* buffer, size_t buffer_size);


#endif /* socket_h */
