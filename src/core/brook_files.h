//
//  brook_files.h
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#ifndef brook_files_h
#define brook_files_h

#include "brook_config.h"

FILE* brook_open_file(char* file_name, char* action);
FILE* brook_open_file_str(brook_str_t file_name, char* action);

char* brook_read_file(FILE* file);
void  brook_close_file (FILE* file);
int   brook_get_files_from_dir(const char* dir);

#endif /* brook_files_h */
