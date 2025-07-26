//
//  brook.c
//  http-c-broker
//
//  Created by David Xavier on 25/07/2025.
//

#include "brook.h"

int
main(int argc, const char * argv[]) {


    // load configuration //
    load_envirmont();

    // init regex //

    // start master process OR single process //

    return 0;

}

int
load_envirmont () {

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
    printf("content: %s\n", data);
    
    return BROOK_OK;
}
