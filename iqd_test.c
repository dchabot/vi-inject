/* Copyright 2025 Osprey DCS
 * All rights reserved
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "iqd_test.h"

static FILE* logf;
static int sock;
static struct sockaddr_in sockaddr;

void lib_init(void) {
    /* FIXME: can't assume success here */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock) {
        sockaddr.sin_family = AF_INET;
        inet_pton(AF_INET, "10.139.4.51", &(sockaddr.sin_addr));
        sockaddr.sin_port = htons(24742);
    }

    /* FIXME: source path from env, then default */
    logf = fopen("/home/admin/iqd_test.log", "a");
    fprintf(logf, "# %s loaded\n", __FILE__);
}

void lib_fini(void) {
    fprintf(logf, "# %s unloading\n", __FILE__);
    close(sock);
    fclose(logf);
}

void print_info(unsigned long int D) {
    uint32_t card, rec, samp;
    int32_t I, Q;

    card = CARD_IDX(D);
    rec = REC_IDX(D);
    samp = SAMP_IDX(D);
    I = S_I(D);
    Q = S_Q(D);

    fprintf(logf, "# word = 0x%lx\n", D);
    fprintf(logf, "# card=%u rec=%u samp=%u I=%i Q=%i\n", card,rec,samp,I,Q );
}

int consume(unsigned long int* data,  unsigned int len) {

    int rc = 0;
    fprintf(logf, "%s: \n", __func__);
    
    for(int i = 0; i < len; i++) {

        if(i == 0) {
            print_info(data[i]);
        }

        if(i == (len - 1)) {
            print_info(data[i]);
        }
    }

    ssize_t nbytes = sendto(sock, data, len * sizeof(unsigned long), 0,
                            (const struct sockaddr *)&sockaddr, sizeof(sockaddr));
    if(nbytes != len*sizeof(unsigned long)) {
        fprintf(logf, "# Error: sent %ld of %ld bytes\n", nbytes, len*sizeof(unsigned long));
    }

    return rc;
}
