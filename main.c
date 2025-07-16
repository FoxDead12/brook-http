//
//  main.c
//  http-c-broker
//
//  Created by David Xavier on 24/06/2025.
//

/*

 Http server to handle request to comunicate with database. The server will handle two tipes of request, direct request to database and the second
 execute controll to execute custom logic. The arquiteture of sofware will be event loop, to handle async logic.
 Using epoll to linux and kqueue to mac os.

    -> Direct comunication to database like json-api

    -> Controllers
        . Logic of controller will be execute in another trhead and the main thread will wait for response

 */

#include <stdio.h>
#include <unistd.h>

#include "src/helpers/types/types.h"
#include "src/worker/worker.h"
#include "src/socket/socket.h"
#include "src/helpers/files/files.h"
#include "src/helpers/json/json.h"

#define CONFIG_FILE "config/http.config.json"

int main(int argc, const char * argv[]) {

    http_main_struct sv_conf;
    sv_conf.pid     =  getpid();
    sv_conf.conf    =  NULL;
    sv_conf.socket  =  0;
    sv_conf.port    =  3001;
    sv_conf.worker_processes = 2;

    // read configuration file
    if ( read_json_file(&sv_conf.conf, CONFIG_FILE) != 0 ) {
        return HTTP_ERROR;
    }

    // init socket
    sv_conf.port              =  json_get_int(sv_conf.conf, "port", sv_conf.port);
    sv_conf.worker_processes  =  json_get_int(sv_conf.conf, "worker_processes", sv_conf.worker_processes);
    sv_conf.workers           =  malloc(sizeof(http_worker_struct) * sv_conf.worker_processes);
    sv_conf.socket            =  socket_init(sv_conf.port);
    
    int r = regcomp(&sv_conf.regex_header, json_get_str(sv_conf.conf, "regex_http_header", http_str("")).data, REG_EXTENDED);
    if (r) {
        fprintf(stderr, "Regex invalido: %s\n", json_get_str(sv_conf.conf, "regex_http_header", http_str("")).data);
        return 1;
    }
    
        
    
#if DEBUG

    worker_event_loop(&sv_conf);

#else

    // can bee more than one process worker
    // init process worker
    init_workers_processes(&sv_conf);

    // await for children processes
    while (1) {

        int status;
        pid_t child_pid = wait(&status);

        if ( child_pid >= 0 ) {
            worker_died(&sv_conf, child_pid);
        }

    }

#endif

    return 0;
}


