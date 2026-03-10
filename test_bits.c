#include <stdio.h>
#include <stdint.h>
#include "iqd_test.h"


int main(void) {
    uint64_t dw = (0x0FUL<<60) | (0x03UL << 50) | (0xA0UL << 40) | ((int32_t)(-1025) << 20) | ((int32_t)(10));
    uint32_t card, rec, samp;
    int32_t I, Q;

    config("10.139.4.51", 5555u, NULL);

    card = CARD_IDX(dw);
    rec = REC_IDX(dw);
    samp = SAMP_IDX(dw);
    I = S_I(dw);
    Q = S_Q(dw);

    printf("word = 0x%lx\n", dw);
    printf("card=%u rec=%u samp=%u I=%i Q=%i\n", card, rec,samp, I, Q);

    // make some easily verifiable data
    uint64_t data[(1<<15)] = {0};
    size_t dlen = sizeof(data)/sizeof(data[0]);
    for(int i=0; i<dlen; i++)
       data[i] = i;

    return consume(data, dlen);
}
