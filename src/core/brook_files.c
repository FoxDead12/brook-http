//
//  brook_files.c
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#include "brook_files.h"

FILE*
brook_open_file(char* file_name, char* action) {
    FILE* fp;
    fp = fopen(file_name, action);
    return fp;
}

char*
brook_read_file(FILE* file) {

    fseek(file, 0, SEEK_END); // make file pointer to end of file
    size_t len = ftell(file); // get lenght of file
    fseek(file, 0, SEEK_SET); // back pointer to start of file

    char* content = malloc(len); // store in heap my file content
    fread(content, 1, len, file); // read file to buffer

    return content;
}

void
brook_close_file (FILE* file) {
    fclose(file);
}
