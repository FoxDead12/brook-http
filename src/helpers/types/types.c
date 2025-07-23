//
//  types.c
//  http-c-broker
//
//  Created by David Xavier on 02/07/2025.
//

#include "types.h"

http_str_s http_str (char* data) {
    
    http_str_s a;
    a.length = (int) strlen(data);
    a.data = data;
    
    return a;
    
}

int str_to_int (http_str_s s) {

    if (s.data == NULL) {
        return 0;
    }
    
    char* tmp = calloc(1, s.length + 1); // add one to null terminator
    
    memcpy(tmp, s.data, s.length);
    
    int i = (int) strtol(tmp, NULL, 10);
    
    free(tmp);
    
    return i;
}

int comp_str_to_str (http_str_s a, http_str_s b) {
    
    if (a.length != b.length) {
        return 1;
    }
    
    for (int i = 0; i < a.length; i++) {
        
        if (*(a.data + i) != *(b.data + i)) {
            return 1;
        }
        
    }
    
    return 0;
}

char* join_array (char** list, size_t size, const char* separator, int free_list) {
    
    char* joined = NULL;
    int joined_size = 0;
    size_t separator_size = strlen(separator);
    
    for (int i = 0; i < size; i++) {
        
        size_t len = strlen(list[i]);
        void* tmp = realloc(joined, joined_size + len + separator_size + 1);
        
        joined = tmp;
        
        if (i != 0) {
            strcpy(joined + joined_size, separator);
            joined_size += separator_size;
        }
        
        strcpy(joined + joined_size, list[i]);
        joined_size += len;
        
        if (free_list == 0) {
            free(list[i]);
        }
        
    }
    
    joined[joined_size + 1] = '\0';
    
    return joined;
}
