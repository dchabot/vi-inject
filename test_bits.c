#include <stdio.h>
#include <stdint.h>
#include "iqd_test.h"


int main(void) {
    uint64_t data = (0x0FUL<<60) | (0x03UL << 50) | (0xA0UL << 40) | ((int32_t)(-1025) << 20) | ((int32_t)(10));
    uint32_t card, rec, samp;
    int32_t I, Q;

    card = CARD_IDX(data);
    rec = REC_IDX(data);
    samp = SAMP_IDX(data);
    I = S_I(data);
    Q = S_Q(data);

    printf("word = 0x%lx\n", data);
    printf("card=%u rec=%u samp=%u I=%i Q=%i\n", card,rec,samp,I,Q );
    return 0;
}
