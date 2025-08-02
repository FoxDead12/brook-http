//
//  brook_string.h
//  http-c-broker
//
//  Created by David Xavier on 25/07/2025.
//

#ifndef brook_string_h
#define brook_string_h

#include "brook_config.h"

typedef struct brook_str_s brook_str_t;
struct brook_str_s {
    size_t     len;
    u_char*    data;
};

#define brook_string(str)    { sizeof(str) - 1, (u_char *) str }
#define brook_null_string    { 0, NULL }
#define brook_str_null(str)  { (str)->len = 0, (str)->data = NULL }

#define brook_tolower(c)     (u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define brook_toupper(c)     (u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

#define brook_strncmp(s1, s2, n) strncmp((const char *) s1, (const char *) s2, n)

#endif /* brook_string_h */
