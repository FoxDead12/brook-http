//
//  brook_connection.c
//  http-c-broker
//
//  Created by David Xavier on 29/07/2025.
//

#include "brook_connection.h"

brook_connection_t*
brook_create_connection (brook_config_t* conf) {
    struct sockaddr_in client_addr;
    int client_socket = brook_socket_accept(conf->socket, &client_addr);
    
    if (client_socket <= 0) {
        return NULL;
    }
    
    brook_connection_t* connection = malloc(sizeof(brook_connection_t));
    connection->conf = conf;
    connection->socket = client_socket;
    connection->port = ntohs(client_addr.sin_port);
    inet_ntop(AF_INET, &(client_addr.sin_addr), connection->ip, INET_ADDRSTRLEN);
    connection->state = READING_SOCKET_MESSAGE;
    
    connection->buff = malloc(sizeof(brook_chain_t));
    connection->buff->buf = brook_create_buffer(4096); // THIS VALUE CAN COME FROM SERVER CONFIG
    connection->buff->next = NULL;
    
    return connection;
}

int
brook_read_message_connection (brook_connection_t* connection) {
    
    brook_buffer_t* buf;
    
    buf = connection->buff->buf;
    size_t n = buf->size - buf->len;
    
    size_t bytes = brook_socket_read(connection->socket, (buf->start + buf->len), n);
    
    if (bytes == BROOK_ERROR) {
        return BROOK_ERROR;
    }
    
    // now in buffer we has to read the request data, basicly parse request
    
    return BROOK_OK;
}
