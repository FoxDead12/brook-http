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
    
    if (client->header.data.lenght > 0) {
        free(client->header.data.start);
    }
    
    free(client);
    
    return 0;
    
}

int socket_new_message (http_request_struct* client) {
    
    // Check if is first time reading from socket or not

    if (client->header.data.lenght == 0) {
        
        // TODO: when get error handle header need close conection and return a response
        if (socket_message_http_header(client) == 1) {
            socket_disconect_connection(client);
            return 1;
        }
        
        set_headers_of_request(client);
        
        // at this point need know, need await for more messages, or not;
        
        if (comp_str_to_str(client->header.method, http_str("OPTIONS")) == 0) {
            return 0;
        }
        
        if (comp_str_to_str(client->header.method, http_str("GET")) == 0 || comp_str_to_str(client->header.method, http_str("DELETE")) == 0) {
            handle_request(client);
            return 0;
        }
        
        if (comp_str_to_str(client->header.method, http_str("POST")) == 0 || comp_str_to_str(client->header.method, http_str("PATCH")) == 0) {
            
            // TODO: need return error, the method post and patch need contain data, if is null need send empty json '{}'
            if (client->header.content_length == 0) {
                socket_disconect_connection(client);
                return 1;
            }
            
            // check if content-lenght is valid
            int max_body_size = json_get_int(client->server_config->conf, "body_max_size", 100000000); // 100MB default max request
            
            // TODO: need return error, body pass limit
            if (client->header.content_length > max_body_size) {
                socket_disconect_connection(client);
                return 1;
            }
            
            // check if header is in buffer memory
            char* body = client->header.data.end + 4; // jump for positions in pointer do jump '\r\n\r\n'
            int body_size = (int) strlen(body);
            
            // check if we has the entire body stored
            if (body_size == client->header.content_length) {
                client->body.data = body;
                client->body.length = body_size;
                handle_request(client);
                return 0;
            }
            
        }
                
    } else {
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
