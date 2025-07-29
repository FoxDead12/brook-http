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
#endif /* brook_socket_h */
