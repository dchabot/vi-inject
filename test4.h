/*
 * Bit-packed U64:
    63:60 Card Number (1-12), 4-bit unsigned
    59:46 Record Index, 14-bit unsigned
    45:36 Sample Index, 10-bit unsigned
    35:18 ADC-Q, 18-bit signed
    17:0 ADC-I, 18-bit signed
*/
#define ADC_MASK ((1<<2) - 1)
#define REC_MASK ((1<<14) - 1)
#define SIDX_MASK ((1<<10) - 1)
#define DATA_MASK ((1<<18) - 1)

#define ADC_IDX(D) (D >> 60) & ADC_MASK
#define REC_IDX(D) (D >> 46) & REC_MASK
#define SAMP_IDX(D) (D >> 36) & SIDX_MASK

#define S_I(D) (D >> 18) & DATA_MASK
#define S_Q(D) (D & DATA_MASK)

void print_info(unsigned long int D);
int init(long unsigned int* data, unsigned int len);

void lib_init(void) __attribute__((constructor));
void lib_fini(void) __attribute__((destructor));
