//
//  brook_connection.c
//  http-c-broker
//
//  Created by David Xavier on 29/07/2025.
//

#include "brook_connection.h"
#include "brook_core.h"
#include "../http/brook_http.h"

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
    connection->state = READING_SOCKET_MESSAGE;
    {
        connection->port = ntohs(client_addr.sin_port);
        inet_ntop(AF_INET, &(client_addr.sin_addr), connection->ip, INET_ADDRSTRLEN);
    }
    {
        connection->buff = malloc(sizeof(brook_chain_t));
        connection->buff->next = NULL;
        connection->buff->buf = malloc(sizeof(brook_buffer_t));
        connection->buff->buf->start = calloc(1, conf->http.buffers_size + 1); // clean all memory in buffer
        connection->buff->buf->pos = connection->buff->buf->start;
        connection->buff->buf->size = conf->http.buffers_size;
    }
    {
        connection->http = malloc(sizeof(brook_http_t));
        connection->http->state = READING_HEADER;
    }
    return connection;
}

int
brook_close_connection (brook_connection_t* connection) {
    
    {
        free(connection->buff->buf->start);
        free(connection->buff->buf);
        free(connection->buff);
    }
    {
        free(connection->http);
    }
    close(connection->socket);
    {
        free(connection);
    }
    
    return BROOK_OK;
}


int
brook_read_message_connection (brook_connection_t* connection) {
    
    brook_buffer_t* buf = connection->buff->buf;
    size_t len_can_red =  buf->size - buf->len;
    
    size_t bytes = brook_socket_read(connection->socket, buf->pos, len_can_red);

    if (bytes == BROOK_ERROR) {
        return BROOK_ERROR;
    }
    
    buf->len += bytes;
    buf->pos += bytes;
    
    if (brook_http_parse(connection) == BROOK_ERROR) {
        return BROOK_ERROR;
    }
    
    return BROOK_OK;
}
