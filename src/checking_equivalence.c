#include <stdio.h>

#ifndef GENKAT_TEST
#include "demo_system.h"
#endif

#include "randombytes.h"
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>  // for variable arguments
#include "stdlib.h"

#include "api.h"

#include "crypto_aead.h"

#include "ascon.h"
#include "shares.h"
#include "word.h"

#include "bendian.h"
#include "forceinline.h"
#include "interleave.h"

#include "permutations.h"

#include "print_simulation.h"
#include "interleave.h"

#ifdef GENKAT_TEST
int _main() {
#else
int main() {
#endif
    srand(RANDOM_SEED);
    puts("RANDOM_SEED = ");
    putdec(RANDOM_SEED);
    puts("\n");

    unsigned char n[CRYPTO_NBYTES] = {0};
    randombytes(n, CRYPTO_NBYTES);
    print_char_array_in_hex_with_name("n", n, CRYPTO_NBYTES);

    unsigned char k[CRYPTO_KEYBYTES] = {0};
    randombytes(k, CRYPTO_KEYBYTES);
    print_char_array_in_hex_with_name("k", k, CRYPTO_KEYBYTES);


#define _alen 16
#define _mlen 32
#define _clen (_mlen + CRYPTO_ABYTES)

    unsigned long long alen = _alen;
    unsigned long long mlen = _mlen;
    unsigned long long clen = _clen;

    unsigned char a[_alen] = {0};
    randombytes(a, _alen);
    print_char_array_in_hex_with_name("a", a, _alen);

    unsigned char m[_mlen] = {0};
    randombytes(m, _mlen);
//    uint64_t test = 0xDEADCAFEBEEFF00D;
    uint64_t test = 0xFFFFFFFF;
    memcpy(m, &test, 4);
    print_char_array_in_hex_with_name("m", m, _mlen);

    unsigned char c[_clen];
    int result = 0;

    mask_m_uint32_t ms;
//    generate_shares_with_print((uint32_t *) &ms, NUM_SHARES_M, m, ASCON_AEAD_RATE);
    ascon_state_t s = {0};
    print_state("test_state", &s);

    s.x[0].s[0].w[0] = 0xDEADBEEF;
    s.x[0].s[1].w[0] = 0x00001E3F;
    print_state("test_state", &s);

    uint64_t test2 = 0x8000000000000001;
    print_char_array_in_hex_with_name("test2   ", (uint8_t*)&test2, 8);
    uint64_t test2_bi = interleave64(test2);
    print_char_array_in_hex_with_name("test2_bi", (uint8_t*)&test2_bi, 8);

    uint64_t test3 = 0x4000000000000002;
    print_char_array_in_hex_with_name("test3   ", (uint8_t*)&test3, 8);
    uint64_t test3_bi = interleave64(test3);
    print_char_array_in_hex_with_name("test3_bi", (uint8_t*)&test3_bi, 8);

    uint32_t e = 0;
    uint32_t o = 0;
    uint32_t lo = 0x00000002;
    uint32_t hi = 0x40000000;
    BD(e,o,lo,hi);
    print_char_array_in_hex_with_name("e", (uint8_t*)&e, 4);
    print_char_array_in_hex_with_name("o", (uint8_t*)&o, 4);


    if (result == 0) {
        puts("result = 0, succes!!\n");
    } else {
        puts("result != 0, fail!!\n");
    }

    return result;
}
