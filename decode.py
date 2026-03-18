import sys
import numpy as np
import matplotlib.pyplot as plt

'''
   Bit-packed U64:
    63:60 Card Number (1-12), 4-bit unsigned
    59:50 Record Index, 10-bit unsigned
    49:40 Sample Index, 10-bit unsigned
    39:20 ADC-Q, 20-bit signed
    19:0 ADC-I, 20-bit signed
''' 
#define CARD_MASK ((1<<4) - 1)
_card_mask = (1<<4) - 1
#define REC_MASK ((1<<10) - 1)
_rec_mask = (1<<10) - 1
#define SIDX_MASK ((1<<10) - 1)
_sidx_mask = _rec_mask
#define DATA_MASK ((1<<20) - 1)
_data_mask = (1<<20) - 1

#define CARD_IDX(D) (D >> 60) & CARD_MASK
def card_idx(d):
    return np.uint64((d >> 60) & _card_mask)
#define REC_IDX(D) (D >> 50) & REC_MASK
def rec_idx(d):
    return np.uint64((d >> 50) & _rec_mask)
#define SAMP_IDX(D) (D >> 40) & SIDX_MASK
def samp_idx(d):
    return np.uint64((d >> 40) & _sidx_mask)

#define S_I(D) (((int32_t)((D >> 20) & DATA_MASK) << 12) >> 12)
def samp_i(d):
    return np.int64(np.int32((d >> 20) & _data_mask) << 12) >> 12

#define S_Q(D) (((int32_t)(D & DATA_MASK) << 12) >> 12)
def samp_q(d):
    return np.int64(np.int32((d & _data_mask) << 12)) >> 12

def print_info(B):
    c,r,s,i,q = card_idx(B), rec_idx(B), samp_idx(B), samp_i(B), samp_q(B)
    print(f'B=0x{B:016x}, {c=}, {r=}, {s=}, {i=}, {q=}')


if __name__ == '__main__':
    with open(sys.argv[1], 'rb') as F:
        B = F.read()
        buf = np.frombuffer(B, dtype='<u8')
        print(f'{buf.size=} words, {buf.size * 8} bytes')

        for n in range(25):
            print_info(buf[n])
        
        if len(sys.argv) > 2:
            I = [ ]
            Q = [ ]
            for w in buf[:int(sys.argv[2])]:
                if card_idx(w) == int(sys.argv[3]):
                    I.append(samp_i(w))
                    Q.append(samp_q(w))

            plt.plot(I, label=f'I')
            plt.plot(Q, label=f'Q')
            plt.legend()
            plt.show()
