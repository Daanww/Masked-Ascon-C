#include "api.h"
#include "ascon.h"
#include "permutations.h"
#include "demo.h"
#include "print_simulation.h"

void ascon_initaead(ascon_state_t* s, const mask_key_uint32_t* k,
                    const mask_npub_uint32_t* n) {
    word_t K1, K2;
    /* randomize the initial state */
    s->x[5] = MZERO(NUM_SHARES_KEY);
    s->x[0] = MZERO(NUM_SHARES_KEY);
//    print_state("randomized initial state", s);
    /* set the initial value */
    s->x[0].s[0].w[0] ^= 0x08220000;
    s->x[0].s[0].w[1] ^= 0x80210000;
//    print_state("set the initial value", s);

    /* set the nonce */
    s->x[3] = MLOAD((uint32_t*)n, NUM_SHARES_NPUB);
    s->x[4] = MLOAD((uint32_t*)(n + 2), NUM_SHARES_NPUB);
//    print_state("set the nonce", s);
    /* first key xor */
    s->x[1] = K1 = MLOAD((uint32_t*)k, NUM_SHARES_KEY);
    s->x[2] = K2 = MLOAD((uint32_t*)(k + 2), NUM_SHARES_KEY);
//    print_state("init 1st key xor", s);
    /* compute the permutation */
    P(s, ASCON_PA_ROUNDS, NUM_SHARES_KEY);
//    print_state("compute the permutation", s);
    /* second key xor */
    s->x[3] = MXOR(s->x[3], K1, NUM_SHARES_KEY);
    s->x[4] = MXOR(s->x[4], K2, NUM_SHARES_KEY);
//    print_state("init 2nd key xor", s);
}

void ascon_adata(ascon_state_t* s, const mask_ad_uint32_t* as, uint64_t alen, unsigned char** a) {
    /* process one block (64 bits) at a time */
    if (alen) {
        /* full associated data blocks */
        while (alen >= ASCON_AEAD_RATE) {
            generate_shares((uint32_t*)as, NUM_SHARES_AD, *a, ASCON_AEAD_RATE);
            word_t a_word = MLOAD((uint32_t*)as, NUM_SHARES_AD);
            s->x[0] = MXOR(s->x[0], a_word, NUM_SHARES_AD);
            printstate("absorb adata", s, NUM_SHARES_AD);
            P(s, ASCON_PB_ROUNDS, NUM_SHARES_AD);
            alen -= ASCON_AEAD_RATE;
            *a += ASCON_AEAD_RATE;
        }
        /* final associated data block */
        s->x[0].s[0].w[1] ^= 0x80000000 >> (alen * 4);
        if (alen) {
            generate_shares((uint32_t*)as, NUM_SHARES_AD, *a, alen);
            word_t a_word = MLOAD((uint32_t*)as, NUM_SHARES_AD);
            s->x[0] = MXOR(s->x[0], a_word, NUM_SHARES_AD);
        }
        printstate("pad adata", s, NUM_SHARES_AD);
        P(s, ASCON_PB_ROUNDS, NUM_SHARES_AD);
    }
    /* domain separation */
    s->x[4].s[0].w[0] ^= 1;
    printstate("domain separation", s, NUM_SHARES_AD);
}


void ascon_encrypt(ascon_state_t* s, mask_c_uint32_t* cs,
                   const mask_m_uint32_t* ms, uint64_t mlen, unsigned char** c, unsigned char** m) {
    /* process one block (64 bits) at a time */
    /* full plaintext blocks */
    while (mlen >= ASCON_AEAD_RATE) {
        generate_shares((uint32_t*)ms, NUM_SHARES_M, *m, ASCON_AEAD_RATE);
        word_t m_word = MLOAD((uint32_t *) ms, NUM_SHARES_M);
        s->x[0] = MXOR(s->x[0], m_word, NUM_SHARES_M);
        MSTORE((uint32_t *) cs, s->x[0], NUM_SHARES_C);
        combine_shares(*c, ASCON_AEAD_RATE, (uint32_t *) cs, NUM_SHARES_C);
        printstate("absorb plaintext", &s, NUM_SHARES_M);
        P(s, ASCON_PB_ROUNDS, NUM_SHARES_M);
        mlen -= ASCON_AEAD_RATE;

        *m += ASCON_AEAD_RATE;
        *c += ASCON_AEAD_RATE;
    }
    /* final plaintext block */
    s->x[0].s[0].w[1] ^= 0x80000000 >> (mlen * 4);
    if (mlen) {
        generate_shares((uint32_t*)ms, NUM_SHARES_M, *m, mlen);
        word_t m_word = MLOAD((uint32_t *) ms, NUM_SHARES_M);
        s->x[0] = MXOR(s->x[0], m_word, NUM_SHARES_M);
        MSTORE((uint32_t *) cs, s->x[0], NUM_SHARES_C);
        combine_shares(*c, mlen, (uint32_t *) cs, NUM_SHARES_C);

        *c += mlen;
    }
    printstate("pad plaintext", &s, NUM_SHARES_M);

}

void ascon_decrypt(ascon_state_t* s, mask_m_uint32_t* ms,
                   const mask_c_uint32_t* cs, uint64_t clen, unsigned char** m, unsigned char** c) {
    /* process one block (64 bits) at a time */
    while (clen >= ASCON_AEAD_RATE) {
        generate_shares((uint32_t*)cs, NUM_SHARES_C, *c, ASCON_AEAD_RATE);
        word_t c_word = MLOAD((uint32_t *) cs, NUM_SHARES_C);
        s->x[0] = MXOR(s->x[0], c_word, NUM_SHARES_C);
        MSTORE((uint32_t *) ms, s->x[0], NUM_SHARES_M);
        combine_shares(*m, ASCON_AEAD_RATE, (uint32_t *) ms, NUM_SHARES_M);
        s->x[0] = c_word;
        printstate("insert ciphertext", s, NUM_SHARES_M);
        P(s, ASCON_PB_ROUNDS, NUM_SHARES_M);
        clen -= ASCON_AEAD_RATE;

        *m += ASCON_AEAD_RATE;
        *c += ASCON_AEAD_RATE;
    }
    /* final ciphertext block */
    s->x[0].s[0].w[1] ^= 0x80000000 >> (clen * 4);
    if (clen) {
        generate_shares((uint32_t*)cs, NUM_SHARES_C, *c, clen);
        word_t c_word = MLOAD((uint32_t *) cs, NUM_SHARES_C);
        s->x[0] = MXOR(s->x[0], c_word, NUM_SHARES_C);
        MSTORE((uint32_t *) ms, s->x[0], NUM_SHARES_M);
        combine_shares(*m, clen, (uint32_t *) ms, NUM_SHARES_M);
        word_t mask = MMASK(s->x[5], clen);
        s->x[0] = MXORAND(c_word, mask, s->x[0], NUM_SHARES_M);
        s->x[5] = MREUSE(s->x[5], 0, NUM_SHARES_M);

        *c += clen;
    }
    printstate("pad ciphertext", s, NUM_SHARES_M);

}


void ascon_final(ascon_state_t* s, const mask_key_uint32_t* k) {
    word_t K1, K2;
    K1 = MLOAD((uint32_t*)k, NUM_SHARES_KEY);
    K2 = MLOAD((uint32_t*)(k + 2), NUM_SHARES_KEY);
    /* first key xor (first 64-bit word) */
    s->x[1] = MXOR(s->x[1], K1, NUM_SHARES_KEY);
    /* first key xor (second 64-bit word) */
    s->x[2] = MXOR(s->x[2], K2, NUM_SHARES_KEY);
    printstate("final 1st key xor", s, NUM_SHARES_KEY);
    /* compute the permutation */
    P(s, ASCON_PA_ROUNDS, NUM_SHARES_KEY);
    /* second key xor (first 64-bit word) */
    s->x[3] = MXOR(s->x[3], K1, NUM_SHARES_KEY);
    /* second key xor (second 64-bit word) */
    s->x[4] = MXOR(s->x[4], K2, NUM_SHARES_KEY);
    printstate("final 2nd key xor", s, NUM_SHARES_KEY);
}

void ascon_settag(ascon_state_t* s, mask_c_uint32_t* cs, unsigned char* c) {
    MSTORE((uint32_t *) cs, s->x[3], NUM_SHARES_C);
    combine_shares(c, ASCON_AEAD_RATE, (uint32_t *) cs, NUM_SHARES_C);
    c += ASCON_AEAD_RATE;
    MSTORE((uint32_t *) cs, s->x[4], NUM_SHARES_C);
    combine_shares(c, ASCON_AEAD_RATE, (uint32_t *) cs, NUM_SHARES_C);
    c += ASCON_AEAD_RATE;
}

/* expected value of x3,x4 for P(0) */
#if ASCON_PB_ROUNDS == 1
static const uint32_t constant[4] = {0x4b000009, 0x1c800003, 0x00000000, 0x00000000};
#elif ASCON_PB_ROUNDS == 2
static const uint32_t constant[4] = {0x5d2d1034, 0x76fa81d1, 0x0cc1c9ef, 0xdb30a503};
#elif ASCON_PB_ROUNDS == 3
static const uint32_t constant[4] = {0xbcaa1d46, 0xf1d0bde9, 0x32c4e651, 0x7b797cd9};
#elif ASCON_PB_ROUNDS == 4
static const uint32_t constant[4] = {0xf7820616, 0xeffead2d, 0x94846901, 0xd4895cf5};
#elif ASCON_PB_ROUNDS == 5
static const uint32_t constant[4] = {0x9e5ce5e3, 0xd40e9b87, 0x0bfc74af, 0xf8e408a9};
#else /* ASCON_PB_ROUNDS == 6 */
static const uint32_t constant[4] = {0x11874f08, 0x7520afef, 0xa4dd41b4, 0x4bd6f9a4};
#endif

void ascon_xortag(ascon_state_t* s, const mask_c_uint32_t* cs, unsigned char* c) {
    /* set x0, x1, x2 to zero */
    s->x[0] = MREUSE(s->x[0], 0, NUM_SHARES_KEY);
    s->x[1] = MREUSE(s->x[1], 0, NUM_SHARES_KEY);
    s->x[2] = MREUSE(s->x[2], 0, NUM_SHARES_KEY);
    /* xor tag to x3, x4 */
    generate_shares((uint32_t*)cs, NUM_SHARES_C, c, ASCON_AEAD_RATE);
    word_t t0 = MLOAD((uint32_t *) cs, NUM_SHARES_C);
    s->x[3] = MXOR(s->x[3], t0, NUM_SHARES_C);
    generate_shares((uint32_t*)cs, NUM_SHARES_C, c + ASCON_AEAD_RATE, ASCON_AEAD_RATE);
    word_t t1 = MLOAD((uint32_t *) cs, NUM_SHARES_C);
    s->x[4] = MXOR(s->x[4], t1, NUM_SHARES_C);
    /* compute P(0) if tags are equal */
    P(s, ASCON_PB_ROUNDS, NUM_SHARES_KEY);
    /* xor expected result to x3, x4 */
    s->x[3].s[0].w[0] ^= constant[0];
    s->x[3].s[0].w[1] ^= constant[1];
    s->x[4].s[0].w[0] ^= constant[2];
    s->x[4].s[0].w[1] ^= constant[3];
}

int ascon_iszero(ascon_state_t* s) {
#if NUM_SHARES_KEY >= 2
    s->x[3].s[0].w[0] ^= ROR32(s->x[3].s[1].w[0], ROT(1));
    s->x[3].s[0].w[1] ^= ROR32(s->x[3].s[1].w[1], ROT(1));
    s->x[4].s[0].w[0] ^= ROR32(s->x[4].s[1].w[0], ROT(1));
    s->x[4].s[0].w[1] ^= ROR32(s->x[4].s[1].w[1], ROT(1));
#endif
#if NUM_SHARES_KEY >= 3
    s->x[3].s[0].w[0] ^= ROR32(s->x[3].s[2].w[0], ROT(2));
  s->x[3].s[0].w[1] ^= ROR32(s->x[3].s[2].w[1], ROT(2));
  s->x[4].s[0].w[0] ^= ROR32(s->x[4].s[2].w[0], ROT(2));
  s->x[4].s[0].w[1] ^= ROR32(s->x[4].s[2].w[1], ROT(2));
#endif
#if NUM_SHARES_KEY >= 4
    s->x[3].s[0].w[0] ^= ROR32(s->x[3].s[3].w[0], ROT(3));
  s->x[3].s[0].w[1] ^= ROR32(s->x[3].s[3].w[1], ROT(3));
  s->x[4].s[0].w[0] ^= ROR32(s->x[4].s[3].w[0], ROT(3));
  s->x[4].s[0].w[1] ^= ROR32(s->x[4].s[3].w[1], ROT(3));
#endif
    uint32_t result;
    result = s->x[3].s[0].w[0] ^ s->x[3].s[0].w[1];
    result ^= s->x[4].s[0].w[0] ^ s->x[4].s[0].w[1];
    result |= result >> 16;
    result |= result >> 8;
    return ((((int)(result & 0xff) - 1) >> 8) & 1) - 1;
}
