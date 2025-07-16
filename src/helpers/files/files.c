//
//  files.c
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#include "files.h"

int read_file (char** data, char* filename) {
    
    FILE *fp = NULL;

    // Open file
    fp = fopen(filename, "r");
    if (fp == NULL) {
        return 1;
    }
        
    // Found size of file
    fseek(fp, 0, SEEK_END);
    long fp_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Store data in memory
    *data = malloc(fp_size);
    fread(*data, 1, fp_size, fp);
    
    return 0;
}

int read_json_file (json_object** conf, char* filename) {
    
    char* file_data = NULL;

    // read file to pointer
    if (read_file(&file_data, filename) != 0) {
        perror(filename);
        return 1;
    }
        
    // convert file data to json pointer
    *conf = json_tokener_parse(file_data);

    free(file_data);
    
    if (*conf == NULL) {
        perror("can't build json object\n");
        return 1;
    }
    
    return 0;
}
