/* Copyright 2026 Osprey DCS
 * All rights reserved
 */
#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif
#ifndef __GNUC__
#  error GCC/clang language extensions used
#endif

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h> // min/max

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//#include <netinet/in.h>

#include "iqd_test.h"
#include "version.h"

#ifndef __linux__
#  error Linux specific
#endif


static FILE* logf;
static int sock;
static struct sockaddr_in sockaddr;
static const char* default_hostaddr = "127.0.0.1";
static const uint16_t default_hostport = 24742;
static const char* default_logfile = "/tmp/iqd.log";
static const size_t block_size = 4*sizeof(uint64_t);
#if ((block_size & (block_size-1)))
    #error
#endif

// estimate available ethernet payload our use to avoid fragmentation
// assume common 1500 bytes, less minimum size for IPv4 and UDP headers.
// Then round down multiple of uint64_t
static const size_t effective_mtu = (1500u - sizeof(struct iphdr) - sizeof(struct udphdr)) &~(block_size-1u);


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

uint32_t config(const char* addr, const unsigned short port, const char* logpath) {
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

    return GIT_HASH;
}

/*
enum struct Err {
    // positive values are errno
    Ok = 0,
    Except = -1,
    Align = -2,
    NoProg = -3,
    TruncUDP = -4,
};
*/
#define OK 0
#define EXCEPT -1
#define ALIGN -2
#define NOPROG -3
#define TRUNCUDP -4

int consume(unsigned long int* data,  unsigned int len) {

    uint8_t *cdata = (uint8_t*)data;
    size_t body_bytes = len * sizeof(unsigned long);

    if(body_bytes%block_size) // required payload granularity
        return ALIGN;

    size_t npacket = body_bytes/effective_mtu;
    if(body_bytes%effective_mtu)
        npacket++;

    fprintf(logf, "# %s(): fragmenting to npackets = %lu\n", __func__, npacket);
    print_info(data[0]);
    print_info(data[len-1]);

    struct mmsghdr mhdrs[npacket]; // variable stack array, GNU extension
    struct iovec vecs[npacket];

    for(size_t n=0; n<npacket; n++) {
        size_t tosend = MIN(body_bytes, effective_mtu);

        vecs[n].iov_base = (void*)cdata;
        vecs[n].iov_len = tosend;

        mhdrs[n].msg_len = 0;
        mhdrs[n].msg_hdr.msg_control = NULL;
        mhdrs[n].msg_hdr.msg_controllen = 0u;
        mhdrs[n].msg_hdr.msg_name = &sockaddr;
        mhdrs[n].msg_hdr.msg_namelen = sizeof(sockaddr);
        mhdrs[n].msg_hdr.msg_iov = &vecs[n];
        mhdrs[n].msg_hdr.msg_iovlen = 1;
        mhdrs[n].msg_hdr.msg_flags = 0;

        body_bytes -= tosend;
        cdata += tosend;
    }

    struct mmsghdr *mnext = mhdrs;

    while(npacket) {
        int ret = sendmmsg(sock, mnext, npacket, 0);
        if(ret<0) {
            perror("sendmmsg");
            return errno;

        } else if(ret==0) {
            return NOPROG; // block call should always queue at least one

        } else {
            // paranoia?
            for(size_t i=0; i<ret; i++) {
                struct mmsghdr mhdr = mhdrs[i];

                if(mhdr.msg_len < mhdr.msg_hdr.msg_iov->iov_len)
                    return TRUNCUDP; // incomplete send with UDP should not be possible
            }
            mnext += ret;
            npacket -= ret;
            // retry
        }
    }

    return OK;
}
