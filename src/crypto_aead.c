#include "crypto_aead.h"

#include "api.h"
#include "ascon.h"
#include "shares.h"

#include "print_simulation.h"

#ifdef SS_VER
#include "hal.h"
#else
#define trigger_high()
#define trigger_low()
#endif

int crypto_aead_encrypt(unsigned char* c, unsigned long long* clen,
                        unsigned char* m, unsigned long long mlen,
                        unsigned char* a, unsigned long long alen,
                        const unsigned char* n,
                        const unsigned char* k) {

    mask_key_uint32_t ks[NUM_WORDS(CRYPTO_KEYBYTES)];
    mask_npub_uint32_t ns[NUM_WORDS(CRYPTO_NBYTES)];
    mask_ad_uint32_t as[NUM_WORDS(ASCON_AEAD_RATE)];
    mask_m_uint32_t ms[NUM_WORDS(ASCON_AEAD_RATE)];
    mask_c_uint32_t cs[NUM_WORDS(ASCON_AEAD_RATE)];

    ascon_state_t s = {0};
    *clen = mlen + CRYPTO_ABYTES;
//    puts("Generating shares for key\n");
//    if(k == NULL) {
//        generate_shares((uint32_t*)ks, NUM_SHARES_KEY, key, CRYPTO_KEYBYTES);
//    }
//    else {
        generate_shares((uint32_t*)ks, NUM_SHARES_KEY, k, CRYPTO_KEYBYTES);
//    }
//    ks[0].shares[0] = 0xDEADBEEF;
//    print_data_shares_in_hex((mask_npub_uint32_t *)ks, NUM_WORDS(CRYPTO_KEYBYTES));
//    puts("\n");
//    puts("Generating shares for nonce\n");
    generate_shares((uint32_t*)ns, NUM_SHARES_NPUB, n, CRYPTO_NBYTES);
//    ns[0].shares[0] = 0x00000000;
//    ns[0].shares[1] = 0x01010101;
//    ns[1].shares[0] = 0x10101010;
//    ns[1].shares[1] = 0x11111111;
//    ns[2].shares[0] = 0x20202020;
//    ns[2].shares[1] = 0x21212121;
//    ns[3].shares[0] = 0x30303030;
//    ns[3].shares[1] = 0x31313131;
//    print_data_shares_in_hex(ns, NUM_WORDS(CRYPTO_NBYTES));


    trigger_high();


    ascon_initaead(&s, ks, ns);

    ascon_adata(&s, as, alen, &a);

    ascon_encrypt(&s, cs, ms, mlen, &c, &m);

    ascon_final(&s, ks);

    ascon_settag(&s, cs, c);


    trigger_low();

    return 0;
}


int crypto_aead_decrypt(unsigned char* m, unsigned long long* mlen,
                        unsigned char* c,
                        unsigned long long clen, unsigned char* a,
                        unsigned long long alen, const unsigned char* n,
                        const unsigned char* k) {
    if (clen < CRYPTO_ABYTES) return -1;

    mask_key_uint32_t ks[NUM_WORDS(CRYPTO_KEYBYTES)];
    mask_npub_uint32_t ns[NUM_WORDS(CRYPTO_NBYTES)];
    mask_ad_uint32_t as[NUM_WORDS(ASCON_AEAD_RATE)];
    mask_m_uint32_t ms[NUM_WORDS(ASCON_AEAD_RATE)];
    mask_c_uint32_t cs[NUM_WORDS(ASCON_AEAD_RATE)];

    ascon_state_t s = {0};
    *mlen = clen - CRYPTO_ABYTES;

    generate_shares((uint32_t*)ks, NUM_SHARES_KEY, k, CRYPTO_KEYBYTES);
    generate_shares((uint32_t*)ns, NUM_SHARES_NPUB, n, CRYPTO_NBYTES);

    trigger_high();


    ascon_initaead(&s, ks, ns);


    ascon_adata(&s, as, alen, &a);

    uint64_t clen_new = *mlen;

    ascon_decrypt(&s, ms, cs, clen_new, &m, &c);


    ascon_final(&s, ks);



    ascon_xortag(&s, cs, c);


    int result = ascon_iszero(&s);

    trigger_low();


    return result;
}
