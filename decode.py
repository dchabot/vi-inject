import sys
import argparse
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
    print(f'B:0x{B:016x}, c={c:d}, r={r:d}, s={s:d}, I={i:d}, Q={q:d}')


def main():
    P = argparse.ArgumentParser()
    P.add_argument('-c', '--card', dest='card', type=int, default=0)
    P.add_argument('-l', '--limit', dest='limit', type=int, default=None)
    P.add_argument('-r', '--record', dest='record', type=int, default=None)
    P.add_argument('-v', '--verbose', dest='verbose', action='store_true', default=False)
    P.add_argument('file', type=str, nargs='?', default='')

    args = P.parse_args()

    with open(args.file, 'rb') as F:
        B = F.read()
        buf = np.frombuffer(B, dtype='<u8')
        print(f'{buf.size=} words, {buf.size * 8} bytes')

        Nch = 4
        if args.card:
            I = [[] for _ in range(Nch)]
            Q = [[] for _ in range(Nch)]
            prev_rec = 0
            prev_samp = 0
            ch_idx = 0

            for w in buf[:args.limit]:
                # ignore null data words
                if w == 0:
                    continue

                if card_idx(w) == args.card:
                    r = rec_idx(w)
                    if args.record is not None and r != args.record:
                        continue 
                    s = samp_idx(w)

                    I[ch_idx%Nch].append(samp_i(w))
                    Q[ch_idx%Nch].append(samp_q(w))
                    if args.verbose:
                        print_info(w)
                    ch_idx += 1

            for i in range(Nch):
                print(f'Size of I[{i}] = {len(I[i])}, Q[{i}] = {len(Q[i])}')

            for n in range(Nch):
                #plt.figure()
                if args.record is not None:
                    max_samp = 1 << 9
                    title = f'Card {args.card}, Record {args.record}'
                else:
                    max_samp = None
                    title = None

                plt.subplot(Nch, 1, n + 1)
                plt.plot(I[n][:max_samp], label=f'I{args.card}.{n}')
                plt.plot(Q[n][:max_samp], label=f'Q{args.card}.{n}')

                plt.suptitle(title)
                plt.legend()
            plt.show()


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
