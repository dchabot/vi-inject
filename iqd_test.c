/* Copyright 2025 Osprey DCS
 * All rights reserved
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include "test3.h"


void lib_init(void) {
    openlog(NULL, LOG_NDELAY | LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "%s loaded\n", __FILE__);
}

void lib_fini(void) {
    syslog(LOG_INFO, "%s unloading\n", __FILE__);
    closelog();
}

int init(unsigned long int* data,  unsigned int len) {

    int rc = 0;
    syslog(LOG_INFO, "data: %lu, len: %u\n", data, len);
    
    return rc;

}
