//
//  socket.c
//  http-c-broker
//
//  Created by David Xavier on 24/06/2025.
//

#include "socket.h"

int socket_init (int port) {

    signal(SIGPIPE, SIG_IGN); /* IGNORE SIGPIPE SIGNAL */

    struct sockaddr_in sv_addr;
    sv_addr.sin_family = AF_INET;
    sv_addr.sin_port = htons(port);
    sv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // in this moment only allow local connections, if wuant anyone machine connecet put INADDR_ANY;

    // create socket
    int s = socket(AF_INET, SOCK_STREAM, 0);

    if (s == -1) {
        printf("%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int opt = 1;

    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int)) == -1) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*) &sv_addr, sizeof(sv_addr)) == -1) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (listen(s, 128) == -1) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    // socket stay no blocking async
    fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);

    return s;
}

http_request_struct* socket_new_connection (http_main_struct *conf) {

    int client = accept(conf->socket, NULL, NULL);

    if (client <= 0) {
        return NULL;
    }

    http_request_struct *c = malloc(sizeof(http_request_struct));
    c->socket = client;
    c->server_config = conf;
    
    return c;

}

int socket_disconect_connection (http_request_struct *client) {
    
    close(client->socket);
    free(client);
    
    return 0;
    
}

int socket_new_message (http_request_struct* client) {
    
    // Check if is first time reading from socket or not

    if (client->header.data.lenght == 0) {
        
        // TODO: when get error handle header need close conection and return a response
        if (socket_message_http_header(client) == 1) {
            return 1;
        }
        
        set_headers_of_request(client);
        
        // At this point i has the important headers
    }
    
    return 0;
}

int socket_message_http_header (http_request_struct* client) {
    
    // alloc memory to store header of request
    int buf_size = json_get_int(client->server_config->conf, "header_buffer", 2048);
    client->header.data.start = calloc(1, buf_size + 1);

    // read from socket
    size_t b = recv(client->socket, client->header.data.start, buf_size, 0);
    if (b <= 0) {
        return 1;
    }
    
    // get end of header
    client->header.data.end = strstr(client->header.data.start, "\r\n\r\n");
    if (client->header.data.end == NULL) {
        return 1;
    }
    
    // calculate lenght of request message
    client->header.data.lenght = (int)(client->header.data.end - client->header.data.start);
        
    return 0;
}

