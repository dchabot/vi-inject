/* Copyright 2025 Osprey DCS
 * All rights reserved
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "iqd_test.h"

static FILE* logf;
static int sock;
static struct sockaddr_in sockaddr;
static const char* default_hostaddr = "127.0.0.1";
static const uint16_t default_hostport = 24742;
static const char* default_logfile = "/tmp/iqd.log";


void lib_init(void) {
    /*
    char *envhost = getenv("IQD_ENDPT"), *hostaddr=NULL;
    uint16_t hostport = 0;

    // FIXME: can't assume success here
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock) {
        if(envhost != NULL) {
            hostaddr = strsep(&envhost, ":");
            if(envhost != NULL) {
                // Delim found. envhost now points past delim
                uint32_t port;
                port = strtoul(envhost, NULL, 10);

                if(port < USHRT_MAX && port > 1024)
                    hostport = (uint16_t)port;
                else
                    hostport = 24742;
            }
            else {
                // FIXME: no token (:) found
                ;
            }
        }
        else {
            hostaddr = (char*)default_hostaddr;
            hostport = default_hostport;
        }

        sockaddr.sin_family = AF_INET;
        sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        sockaddr.sin_port = htons(24742);
    }

    char* logpath = getenv("IQD_LOGFILE");
    if(logpath != NULL)
        logf = fopen(logpath, "a");
    else
        logf = fopen(default_logfile, "a");

    fprintf(logf, "# %s loaded. Using ip4 addr: %s:%hd\n",
            __FILE__, hostaddr, hostport);
    */
}

void lib_fini(void) {
    fflush(logf);
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

int config(const char* addr, const unsigned short port, const char* logpath) {
    // FIXME: can't assume success here
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(addr != NULL) {
        sockaddr.sin_addr.s_addr = inet_addr(addr);
    }
    else
        sockaddr.sin_addr.s_addr = inet_addr(default_hostaddr);

    if(port < 1024)
        sockaddr.sin_port = htons(default_hostport);
    else
        sockaddr.sin_port = htons(port);

    if(logpath != NULL)
        logf = fopen(logpath, "a");
    else
        logf = fopen(default_logfile, "a");

    fprintf(logf, "# %s(): using ip4 addr: %s:%hd\n",
            __func__, inet_ntoa(sockaddr.sin_addr), ntohs(sockaddr.sin_port));

    return 0;
}

int consume(unsigned long int* data,  unsigned int len) {

    int rc = 0;
    fprintf(logf, "# %s(): \n", __func__);
    
    for(int i = 0; i < len; i++) {

        if(i == 0) {
            print_info(data[i]);
        }

        if(len > 1 && i == (len - 1)) {
            print_info(data[i]);
        }
    }

    ssize_t nbytes = sendto(sock, data, len * sizeof(unsigned long), 0,
                            (const struct sockaddr *)&sockaddr, sizeof(sockaddr));
    if(nbytes < 0)
        fprintf(logf, "# %s(): sendto() error: %s\n", __func__, strerror(errno));
    else if(nbytes != len*sizeof(unsigned long))
        fprintf(logf, "# %s() error: sent %ld of %ld bytes\n",
                __func__, nbytes, len*sizeof(unsigned long));

    return rc;
}
