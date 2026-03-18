/*
 * Bit-packed U64:
    63:60 Card Number (1-12), 4-bit unsigned
    59:50 Record Index, 10-bit unsigned
    49:40 Sample Index, 10-bit unsigned
    39:20 ADC-Q, 20-bit signed
    19:0 ADC-I, 20-bit signed
 *
 */
#define CARD_MASK (uint64_t)((1<<4) - 1)
#define REC_MASK  (uint64_t)((1<<10) - 1)
#define SIDX_MASK (uint64_t)((1<<10) - 1)
#define DATA_MASK (uint64_t)((1<<20) - 1)

#define CARD_IDX(D) ((uint64_t)D >> 60) & CARD_MASK
#define REC_IDX(D) ((uint64_t)D >> 50) & REC_MASK
#define SAMP_IDX(D) ((uint64_t)D >> 40) & SIDX_MASK

#define S_I(D) (((int32_t)((D >> 20) & DATA_MASK) << 12) >> 12)
#define S_Q(D) (((int32_t)(D & DATA_MASK) << 12) >> 12)

void print_info(unsigned long int D);
uint32_t config(const char* addr, const unsigned short port, const char* logpath);
int consume(long unsigned int* data, unsigned int len);

void lib_init(void) __attribute__((constructor));
void lib_fini(void) __attribute__((destructor));
