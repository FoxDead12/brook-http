//
//  brook_socket.h
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#ifndef brook_socket_h
#define brook_socket_h

#include "brook_core.h"

int brook_init_socket(int port);
int brook_socket_accept(int socket, struct sockaddr_in* client_addr);
size_t brook_socket_read(int socket, u_char* buffer, size_t size);

#endif /* brook_socket_h */
