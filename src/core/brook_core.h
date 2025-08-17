//
//  brook_core.h
//  http-c-broker
//
//  Created by David Xavier on 25/07/2025.
//

#ifndef brook_core_h
#define brook_core_h

#include "brook_config.h"

#define BROOK_OK      0
#define BROOK_ERROR   -1
#define BROOK_DONE    -2
#define BROOK_CONFIG_FILE "../../conf/brook_config.json"
#define BROOK_RESOURCES_DIRECTORY "../../resources"
#define BROOK_GATEKEEPER_DIRECTORY "../../conf/gatekeeper.json"

#include "brook_files.h"
#include "brook_json.h"
#include "brook_socket.h"
#include "brook_process.h"
#include "brook_connection.h"
#include "brook_gatekeeper.h"

#endif /* brook_core_h */
