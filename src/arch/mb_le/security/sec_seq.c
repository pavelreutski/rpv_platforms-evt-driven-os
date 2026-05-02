#include "security/sec_seq.h"

#define LCG_M                  (12345u)
#define LCG_K                  (1103515245u)

#define LCG_SEED_K             (1664525u)
#define LCG_SEED_M             (1013904223u)

#define MAX_SHIFT              (32)

static uint32_t state = 1;

static uint32_t prng_next(void);
static uint8_t prng_nextbyte(void);

void _sec_prngseed(uint32_t seed) {    

    seed *= LCG_SEED_K;
    seed += LCG_SEED_M;

    state = seed;    
}

void _sec_genprngseq(void *seq, const size_t seq_size) {

    size_t i;
    uint8_t min, max;

    uint8_t *seq_blck;

    for (seq_blck = seq, min = 0, max = 0xFF, i = 0; i < seq_size; i++) {

        uint8_t b = prng_nextbyte();

        seq_blck[i] = b;

        min = min <= b ? min : b;
        max = max >= b ? max : b;
    } 

    uint8_t p[UINT8_MAX] = { 0 };
    uint8_t c = ((max - min) >> 1);

    for (i = 0; i < seq_size; i++) {

        uint8_t x = (seq_blck[i] - c);

        size_t p_k = 0;
        int8_t m = (i % 2 == 0) ? 1 : -1;

        while ((p[x]) && (p_k <= UINT8_MAX)) {

            p_k++;
            x = x + m;
        }        

        p[x]++;
        seq_blck[i] = x;
    }
}

static uint32_t prng_next(void) {    

    state *= LCG_K;
    state += LCG_M;

    return state;
}

static uint8_t prng_nextbyte(void) {

    static int shift = MAX_SHIFT;
    static uint32_t buffer = 0;

    if (shift >= MAX_SHIFT) {

        shift = 0;
        buffer = prng_next();
    }

    uint8_t b = (buffer >> shift) & 0xFF;
    shift += 8;

    return b;
}