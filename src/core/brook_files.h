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
char* brook_read_file(FILE* file);
void  brook_close_file (FILE* file);

#endif /* brook_files_h */
