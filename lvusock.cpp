/** Copyright 2025 Osprey DCS
 *  All rights reserved
 */

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif
#ifndef __GNUC__
#  error GCC/clang language extensions used
#endif

#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdio>

#include <stdint.h>

#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef __linux__
#  error Linux specific
#endif

#define LVUS_API __attribute__ ((visibility("default")))

namespace {

const char* default_hostaddr = "127.0.0.1";
const uint16_t default_hostport = 24742;
const char* default_logfile = "/tmp/iqd.log";

constexpr size_t block_size = 4*sizeof(uint64_t);
static_assert((block_size & (block_size-1))==0, "must be power of 2");

// estimate available ethernet payload our use to avoid fragmentation
// assume common 1500 bytes, less minimum size for IPv4 and UDP headers.
// Then round down multiple of uint64_t
constexpr size_t effective_mtu = (1500u - sizeof(iphdr) - sizeof(udphdr)) &~(block_size-1u);

struct LVUSock {
    int sock = -1;
    FILE *logf = NULL;
    struct sockaddr_in sockaddr = {0};

    constexpr LVUSock() {};
    ~LVUSock();

    int setup();
} lvusingle;

enum struct Err {
    // positive values are errno
    Ok = 0,
    Except = -1,
    Align = -2,
    NoProg = -3,
    TruncUDP = -4,
};


int lvu_sendmmsg(const uint8_t* body, uint32_t body_bytes) noexcept
{
    try {
        if(body_bytes%block_size) // required payload granularity
            return (int)Err::Align;

        size_t npacket = body_bytes/effective_mtu;
        if(body_bytes%effective_mtu)
            npacket++;

        struct mmsghdr mhdrs[npacket]; // variable stack array, GNU extension
        struct iovec vecs[npacket];

        for(size_t n=0; n<npacket; n++) {
            auto tosend = std::min(size_t(body_bytes), effective_mtu);

            auto& vec = vecs[n];
            vec.iov_base = (void*)body;
            vec.iov_len = tosend;

            mhdrs[n].msg_len = 0;
            auto& hdr = mhdrs[n].msg_hdr;
            hdr.msg_control = nullptr;
            hdr.msg_controllen = 0u;
            hdr.msg_name = &lvusingle.sockaddr;
            hdr.msg_namelen = sizeof(lvusingle.sockaddr);
            hdr.msg_iov = &vec;
            hdr.msg_iovlen = 1;
            hdr.msg_flags = 0;

            body_bytes -= tosend;
            body += tosend;
        }

        auto mnext = mhdrs;

        while(npacket) {
            auto ret = sendmmsg(lvusingle.sock, mnext, npacket, 0);
            if(ret<0) {
                fprintf(lvusingle.logf, "Error sendmmsg(): %s\n", strerror(errno));
                return errno;

            } else if(ret==0) {
                return (int)Err::NoProg; // block call should always queue at least one

            } else {
                // paranoia?
                for(auto i=0; i<ret; i++) {
                    const auto& mhdr = mhdrs[i];
                    if(mhdr.msg_len < mhdr.msg_hdr.msg_iov->iov_len)
                        return (int)Err::TruncUDP; // incomplete send with UDP should not be possible
                }
                mnext += ret;
                npacket -= ret;
                // retry
            }
        }

        return (int)Err::Ok;
    }catch(const std::exception& e){
        // TODO: how to report?
        (void)e;
        return (int)Err::Except;
    }
}

LVUSock::~LVUSock()
{
    if(sock>=0)
        (void)close(sock);
    //fflush(logf);
    //fprintf(logf, "# %s unloading\n", __FILE__);
    fclose(logf);
}

int LVUSock::setup()
{
    if(sock<0) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock<0)
            throw std::runtime_error("Unable to allocate UDP socket");

        // TODO: set/extend socket buffer size
    }

    return sock;
}
} // namespace

extern "C"
LVUS_API
int config(const char* addr, const unsigned short port, const char* logpath) {
    lvusingle.setup();

    if(addr != NULL) {
        lvusingle.sockaddr.sin_addr.s_addr = inet_addr(addr);
    }
    else
        lvusingle.sockaddr.sin_addr.s_addr = inet_addr(default_hostaddr);

    if(port < 1024)
        lvusingle.sockaddr.sin_port = htons(default_hostport);
    else
        lvusingle.sockaddr.sin_port = htons(port);

    if(logpath != NULL)
        lvusingle.logf = fopen(logpath, "a");
    else
        lvusingle.logf = fopen(default_logfile, "a");

    fprintf(lvusingle.logf, "# %s(): using ip4 addr: %s:%hd\n",
            __func__, inet_ntoa(lvusingle.sockaddr.sin_addr), ntohs(lvusingle.sockaddr.sin_port));

    return 0;
}

int consume(unsigned long int* data,  unsigned int len) {
    return lvu_sendmmsg((const uint8_t*)data, len*sizeof(uint64_t));
}
