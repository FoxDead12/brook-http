//
//  brook_string.c
//  http-c-broker
//
//  Created by David Xavier on 04/08/2025.
//

#include "brook_string.h"
#include "brook_core.h"


int
brook_str_to_int (brook_str_t s) {
    
    if (s.len == 0 || s.data == NULL) return 0;
    
    char* tmp = calloc(1, s.len + 1);
    memcpy(tmp, s.data, s.len);
    
    int value = atoi(tmp);
    free(tmp);
    
    return value;
}
