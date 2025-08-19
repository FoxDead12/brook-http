//
//  brook.c
//  http-c-broker
//
//  Created by David Xavier on 25/07/2025.
//

#include "brook.h"
#include "brook_core.h"

int
main(int argc, const char * argv[]) {

    brook_config_t conf;

    if (brook_create_configuration(&conf) == BROOK_ERROR) {
        return 1;
    }
    conf.socket = brook_init_socket(conf.port);

	brook_resources_generator(&conf);
    brook_gatekeeper_generator(&conf);
	
	// start master process OR single process //
#if DEBUG
    brook_start_single_process(&conf);
#else
    brook_start_main_process(&conf);
#endif

    return 0;

}

int
brook_create_configuration (brook_config_t* conf) {

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
    {
        conf->json = json_tokener_parse(data);
        conf->port = json_get_int("port", conf->json, 0);
        conf->worker_processes = json_get_int("worker_processes", conf->json, 4); // 4 worker process in default
        conf->brook_processes = brook_create_array(conf->worker_processes);
        conf->brook_parent_process = getpid();
    }
    {
        conf->http.timeout = json_get_int("http_request_timeout_ms", conf->json, 1000); // 1 SECOND default
        conf->http.max_body_size = json_get_int("http_request_max_body_size", conf->json, 1048576); // 1MB default size
        conf->http.allow_content_types = json_get_array("http_request_allow_content_type", conf->json);
        conf->http.buffers_size = json_get_int("http_request_buffers_size", conf->json, 4096);
    }
    {
        if(regcomp(&conf->regex.http_line, (char*) json_get_str("regex_http_line", conf->json, (brook_str_t) brook_string("")).data, REG_EXTENDED)) {
            perror("regex of field 'regex_http_line' is invalid\n");
            return BROOK_ERROR;
        }
    }
    {
        conf->resources  = json_object_new_object();
        conf->gatekeeper = NULL;
    }
    
    free(data);

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

int
brook_gatekeeper_generator (brook_config_t* conf) {
    
    // read configuration file
    FILE* file = brook_open_file(BROOK_GATEKEEPER_DIRECTORY, "r");
    if (file == NULL) {
        perror(BROOK_GATEKEEPER_DIRECTORY);
        return BROOK_ERROR;
    }

    // ... convert data of file to json
    char* gatekeeper_file = brook_read_file(file);
    json_object* gatekeeper_json = json_tokener_parse(gatekeeper_file);
    
    // ... build object to manager gatekeeper
    if (brook_gatekeeper_build(conf, gatekeeper_json) == BROOK_ERROR) {
        return BROOK_ERROR;
    }
    
    // ... free data
    free(gatekeeper_file);
    json_object_put(gatekeeper_json);
    
    brook_close_file(file);
            
    return BROOK_OK;
}


int
brook_resources_generator (brook_config_t* conf) {

	const char* dir = BROOK_RESOURCES_DIRECTORY;
	struct dirent* in_file;
	DIR* FD;
	FILE* file;

    // ... check if exist directory
	if (NULL == (FD = opendir(dir))) {
		fprintf(stderr, "Error : Failed to open input directory (%s) - %s\n", dir, strerror(errno));
        return BROOK_ERROR;
	}
	    
    // ... iterate each file in directory
	while ((in_file = readdir(FD))) {
		
		// ... ignore hidden files in unix
		if (!strcmp (in_file->d_name, "."))
			continue;
		if (!strcmp (in_file->d_name, ".."))
			continue;
            
        // ... create file path
        char file_path[2048] = {0};
        snprintf(file_path, 2048, "%s/%s", dir, (u_char*) in_file->d_name);
        
        // ... open file and read
		file = brook_open_file(file_path, "r");
        if (file == NULL) continue;
        char* file_content = brook_read_file(file);
        
        // ... copy content of file to json object
        json_object* json = json_tokener_parse(file_content);
        free(file_content);

        json_object_object_foreach(json, key, val) {
            const char *tmp = json_object_to_json_string(val);
            json_object *copy = json_tokener_parse(tmp);
            json_object_object_add(conf->resources, key, copy);
        }
        json_object_put(json);

		brook_close_file(file);

	}
    
    closedir(FD);
	    
    return BROOK_OK;
}
