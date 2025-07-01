//
//  files.h
//  http-c-broker
//
//  Created by David Xavier on 01/07/2025.
//

#ifndef files_h
#define files_h

#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>

int read_file (char** data, char* filename);
int read_json_file (json_object** conf, char* filename);

#endif /* files_h */
