//
//  types.c
//  http-c-broker
//
//  Created by David Xavier on 02/07/2025.
//

#include "types.h"

int conv_str_to_int (http_str_s s) {

    if (s.data == NULL) {
        return 0;
    }
    
    char* tmp = calloc(1, s.length + 1); // add one to null terminator
    
    memcpy(tmp, s.data, s.length);
    
    int i = (int) strtol(tmp, NULL, 10);
    
    free(tmp);
    
    return i;
}
