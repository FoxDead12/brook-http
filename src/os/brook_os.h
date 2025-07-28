//
//  brook_os.h
//  http-c-broker
//
//  Created by David Xavier on 28/07/2025.
//

#ifndef brook_os_h
#define brook_os_h

#include "../core/brook_core.h"

#ifdef __APPLE__
    #include <sys/event.h>
    #include "brook_kqueue.h"
#endif

#define MAX_EVENTS 128

#endif /* brook_os_h */
