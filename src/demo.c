#include <stdio.h>

#ifndef GENKAT_TEST
#include "demo_system.h"
#endif

//#include "randombytes.h"
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>  // for variable arguments
#include "stdlib.h"

#include "api.h"

#include "crypto_aead.h"

#include "ascon.h"
#include "shares.h"
#include "word.h"

#include "print_simulation.h"
#include "c_rust_interop_test.h"


#ifdef GENKAT_TEST
int _main() {
#else
int main() {
#endif
    srand(RANDOM_SEED);
    //puts("RANDOM_SEED = ");
    //putdec(RANDOM_SEED);
    //puts("\n");
    my_printf("RANDOM_SEED = %d\n", RANDOM_SEED);
    uint32_t interop_test = rust_function();
    my_printf("rust_function() = %d\n", interop_test);
    uint32_t interop_addition = rust_addition(4, 5);
    my_printf("rust_addition() = %d\n", interop_addition);
    char *interop_string = "test";
//    uint32_t interop_count = how_many_characters(interop_string);
//    my_printf("how_many_characters(\"test\") = %d\n", interop_count);


    unsigned char n[CRYPTO_NBYTES] = {0};
    memset(n, 0x11, CRYPTO_NBYTES);
//    randombytes(n, CRYPTO_NBYTES);
    print_char_array_in_hex_with_name("n", n, CRYPTO_NBYTES);

    unsigned char k[CRYPTO_KEYBYTES] = {0};
    memset(k, 0x22, CRYPTO_KEYBYTES);
//    randombytes(k, CRYPTO_KEYBYTES);
    print_char_array_in_hex_with_name("k", k, CRYPTO_KEYBYTES);


#define _alen 8
#define _mlen 16
#define _clen _mlen + CRYPTO_ABYTES

    unsigned long long alen = _alen;
    unsigned long long mlen = _mlen;
    unsigned long long clen = _clen;

    unsigned char a[_alen] = {0};
    memset(a, 0x44, _alen);
//    randombytes(a, _alen);
    print_char_array_in_hex_with_name("a", a, _alen);

    unsigned char m[_mlen] = {0};
    memset(m, 0x88, _mlen);
//    randombytes(m, _mlen);
    print_char_array_in_hex_with_name("m", m, _mlen);

    unsigned char c[_clen];
    int result = 0;




    result |= crypto_aead_encrypt(c, &clen, m, mlen, a, alen, n, k);


    result |= crypto_aead_decrypt(m, &mlen, c, clen, a, alen, n, k);


    if(result == 0) {
        puts("result = 0, succes!!\n");
    } else {
        puts("result != 0, fail!!\n");
    }

    return result;
}
