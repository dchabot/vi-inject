#include <stdio.h>
#include <stdint.h>
#include "iqd_test.h"


int main(void) {
    uint64_t c = (0x0AUL << 60), r = (0x0BUL << 50), s = (0x0CUL << 40);
    uint64_t i = ((-1025UL & ((1<<20) - 1)) << 20), q = (10UL & ((1<<20) - 1));
    uint64_t dw = c | r | s | i | q;
    uint32_t card, rec, samp;
    int32_t I, Q;

    uint32_t vers = config("10.139.4.51", 5555u, NULL);
    printf("version = 0x%x\n", vers);

    card = CARD_IDX(dw);
    rec = REC_IDX(dw);
    samp = SAMP_IDX(dw);
    I = S_I(dw);
    Q = S_Q(dw);

    printf("word = 0x%016lx\n", dw);
    printf("card=%u rec=%u samp=%u I=%i Q=%i\n", card, rec,samp, I, Q);

    // make some easily verifiable data
    uint64_t data[(1<<15)] = {0};
    size_t dlen = sizeof(data)/sizeof(data[0]);
    for(int i=0; i<dlen; i++)
       data[i] = i;

    return consume(data, dlen);
}
