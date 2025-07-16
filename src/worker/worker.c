//
//  worker.c
//  http-c-broker
//
//  Created by David Xavier on 25/06/2025.
//

#include "worker.h"

// INIT TIME METHODS //

int init_workers_processes (http_main_struct* conf) {

    // create children process to handle connections
    for (int i = 0; i < conf->worker_processes; i++) {
        
        pid_t child_pid = init_worker(conf, i);
        
        if (child_pid != HTTP_ERROR) {
            conf->workers[i].pid = child_pid;
        }
        
    }

    return HTTP_OK;
}

pid_t init_worker (http_main_struct* conf, int index) {

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork() can't create process");
        return HTTP_ERROR;
    }

    if (pid == 0) {
        worker_event_loop(conf);
    }

    return pid;
}

int worker_died (http_main_struct* conf, pid_t pid) {

    printf("worker process died %d\n", pid);

    for (int i = 0; i < conf->worker_processes; i++) {
        if (conf->workers[i].pid == pid) {
            pid_t child_pid = init_worker(conf, i);
            
            if (child_pid != HTTP_ERROR) {
                conf->workers[i].pid = child_pid;
            }
        }
    }

    return HTTP_OK;
}


// RUN TIME METHODS //


void worker_event_loop (http_main_struct* conf) {

    // where is necessary to handle kqueue in mac os and epoll in linux
    printf("worker process start %d\n", getpid());
    
    // init worket settings
    http_worker_struct worker;
    worker.pid           =  getpid();
    worker.server        =  conf;
    
    // init setting of db to worker
    worker.pool.conns_number = json_get_int(conf->conf, "db_worker_connections", 4); // connections peer worker
    worker.pool.conns_info   = malloc(sizeof(int) * worker.pool.conns_number);
    worker.pool.conns        = malloc(sizeof(PGconn*) * worker.pool.conns_number); // create connection to database
    init_db_connections(&worker.pool, json_get_str(conf->conf, "db_string_connection", http_str("")));
    
#ifdef __APPLE__

    kqueue_init(&worker);
    
#endif
    
    exit(HTTP_OK);
    
}

int worker_close_connection (http_connection_struct* con) {
    
    close(con->socket);
    free(con->b_header.start);
    free(con);
    
    return HTTP_OK;
}

int worker_accept_new_connection (http_worker_struct* worker, http_connection_struct **con) {
    
    struct sockaddr_in client_addr;
    
    int client_socket = socket_connection(worker->server->socket, (struct sockaddr_in*) &client_addr);
    
    if (client_socket <= 0) {
        return HTTP_DONE; // ignore, when has multi process only one will catch the connection of socket
    }
    
    (*con) = malloc(sizeof(http_connection_struct));
    (*con)->status = 0;
    (*con)->worker = worker;
    (*con)->socket = client_socket;
    (*con)->port   = ntohs(client_addr.sin_port);                                           // store client port
    inet_ntop(AF_INET, &(client_addr.sin_addr), (*con)->ip, INET_ADDRSTRLEN);               // store client ip
    (*con)->b_header.size = json_get_int(worker->server->conf, "headers_buffer", 4) * 1024; // todo passe to worker config, to calculate size
    
    printf("Connection Socket: %d IP: %s Port: %d\n", client_socket, (*con)->ip, (*con)->port);
    
    return HTTP_OK;
    
}

int worker_read_connection (http_connection_struct *con) {
    
    // check if we has the header to be read
    if (con->b_header.length == 0) {
        
        size_t size = con->b_header.size;
        con->b_header.start = calloc(1, size + 1); // add one more case to put '\0', calloc reset all bytes so it's fine
        
        size_t b = socket_read(con->socket, con->b_header.start, size);
        if (b == HTTP_NOT_OK) {
            return HTTP_ERROR;
        }
        
        con->b_header.end = strstr(con->b_header.start, "\r\n\r\n");
        
        if (con->b_header.end == NULL) {
            return HTTP_ERROR;
        }
        
        con->b_header.length = b;
        
        if (request_set_headers(con) == HTTP_ERROR) {
            return HTTP_ERROR;
        }
        
    }
    
    con->status = 1; // update status to build query and send to db
    
    return HTTP_OK;
    
}

int worker_build_and_send_async_query (http_connection_struct* con, char* query, int* socket) {
    
    int index = get_db_free_connection(&con->worker->pool);

    if (index == HTTP_NOT_OK) {
        return HTTP_DONE;
    }
    
    PGconn* db = con->worker->pool.conns[index];
    *socket = PQsocket(db);
    
    // build query
    generate_query_from_request(con);
    
    PQsendQuery(db, query);
    
    con->status = 2; // update status to handle the db response
    
    return HTTP_OK;
}

int worker_read_async_query (http_connection_struct* con, int socket) {
    
    get_db_query_result(con, socket);
    
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Hello, world!";

    write(con->socket, response, strlen(response));
    
    worker_close_connection(con);
    
    return HTTP_OK;
}
