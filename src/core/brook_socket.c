//
//  brook_socket.c
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#include "brook_socket.h"

int
brook_init_socket (int port) {
    signal(SIGPIPE, SIG_IGN);
    
    struct sockaddr_in brook_addr;
    brook_addr.sin_family  = AF_INET;
    brook_addr.sin_port    = htons(port);
    //brook_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // only allow local connections
    brook_addr.sin_addr.s_addr = htonl(INADDR_ANY); // open to internet
    
    int s = socket(AF_INET, SOCK_STREAM, 0);
    
    if (s == -1) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;

    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int)) == -1) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if (bind(s, (struct sockaddr*) &brook_addr, sizeof(brook_addr)) == -1) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1024) == -1) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
    
    return s;
}

int
brook_socket_accept (int socket, struct sockaddr_in* client_addr) {
    socklen_t size = sizeof(struct sockaddr_in);
    return accept(socket, (struct sockaddr*) client_addr, &size);
}
