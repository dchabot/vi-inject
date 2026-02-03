/* Copyright 2025 Osprey DCS
 * All rights reserved
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include "test4.h"


void lib_init(void) {
    openlog(NULL, LOG_NDELAY | LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "%s loaded\n", __FILE__);
}

void lib_fini(void) {
    syslog(LOG_INFO, "%s unloading\n", __FILE__);
    closelog();
}

void print_info(unsigned long int D) {
    uint32_t adc, rec, samp;
    int32_t I, Q;

    adc = ADC_IDX(D);
    rec = REC_IDX(D);
    samp = SAMP_IDX(D);
    I = S_I(D);
    Q = S_Q(D);

    syslog(LOG_INFO, "adc=%u rec=%u samp=%u I=%i Q=%i\n", adc,rec,samp,I,Q );
}

int init(unsigned long int* data,  unsigned int len) {

    int rc = 0;
    syslog(LOG_INFO, "%s: \n", __func__);
    
    for(int i = 0; i < len; i++) {

        if(i == 0) {
            print_info(data[i]);
        }

        if(i == (len - 1)) {
            print_info(data[i]);
        }
    }

    return rc;
}
