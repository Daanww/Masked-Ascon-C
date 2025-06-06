#define CRYPTO_VERSION "1.2.7"
#define CRYPTO_KEYBYTES 16
#define CRYPTO_NBYTES 16
#define CRYPTO_ABYTES 16
#define CRYPTO_NOOVERLAP 1
#define ASCON_AEAD_RATE 8

#ifndef ASCON_ROR_SHARES
#define ASCON_ROR_SHARES 5
#endif

#ifndef NUM_SHARES_M
#define NUM_SHARES_M 3
#endif

#ifndef NUM_SHARES_C
#define NUM_SHARES_C 3
#endif

#ifndef NUM_SHARES_AD
#define NUM_SHARES_AD 3
#endif

#ifndef NUM_SHARES_NPUB
#define NUM_SHARES_NPUB 3
#endif

#ifndef NUM_SHARES_KEY
#define NUM_SHARES_KEY 3
#endif

#ifndef RANDOM_SEED
#define RANDOM_SEED 0
#endif