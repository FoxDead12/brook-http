#ifndef _BROOK_SOCKET_H
#define _BROOK_SOCKET_H

#include "config.h"

int brook_socket(int PORT);
int brook_socket_close(int socket);
int brook_socket_accept(int socket, struct sockaddr* _addr, socklen_t* _addr_len);
size_t brook_socket_recv(int socket, unsigned char* b, size_t size);

#endif
