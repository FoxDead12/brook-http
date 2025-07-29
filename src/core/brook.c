//
//  brook.c
//  http-c-broker
//
//  Created by David Xavier on 25/07/2025.
//

#include "brook.h"

int
main(int argc, const char * argv[]) {

    brook_config_t conf;
    
    if (create_configuration(&conf) == BROOK_ERROR) {
        return 1;
    }
    conf.socket = brook_init_socket(conf.port);

#if DEBUG
    brook_start_single_process(&conf);
#else
    brook_start_main_process(&conf);
#endif
    
    // start master process OR single process //

    return 0;

}

int
create_configuration (brook_config_t* conf) {

    // read configuration file
    FILE* file = brook_open_file(BROOK_CONFIG_FILE, "r");
    if (file == NULL) {
        perror(BROOK_CONFIG_FILE);
        return BROOK_ERROR;
    }

    char* data = brook_read_file(file);
    brook_close_file(file);
    
    if (data == NULL) {
        perror(BROOK_CONFIG_FILE);
        return BROOK_ERROR;
    }

    // now load the file content to json object
    conf->json = json_parse(data);
    conf->port = json_get_int("port", conf->json, 0);
    conf->worker_processes = json_get_int("worker_processes", conf->json, 4); // 4 worker process in default
    conf->http.timeout = json_get_int("http_request_timeout_ms", conf->json, 1000); // 1 SECOND default
    conf->http.max_body_size = json_get_int("http_request_max_body_size", conf->json, 1048576); // 1MB default size
    conf->http.allow_content_types = json_get_array("http_request_allow_content_type", conf->json);
    conf->brook_processes = brook_create_array(conf->worker_processes, sizeof(int*));
    conf->brook_parent_process = getpid();
    
    if (conf->port == 0) {
        perror("configuration missing 'port' in json file configuration -> integer\n");
        return BROOK_ERROR;
    }
    if (conf->http.allow_content_types->size == 0) {
        perror("configuration missing 'http_request_allow_content_type' in json file configuration -> array\n");
        return BROOK_ERROR;
    }
    
    for (int i = 0; i < conf->worker_processes; i++) {
        conf->brook_processes->data[i] = (void*) -1;
    }
            
    return BROOK_OK;
}
