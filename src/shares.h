#ifndef SHARES_H_
#define SHARES_H_

#include <stdint.h>

#include "api.h"
#include "config.h"
#include "randombytes.h"

#define NUM_WORDS(len) ((((len) + 7) / 8) * 2)
#define ROT(i) (((i) * (ASCON_ROR_SHARES)) & 31)
#define RND(rnd) randombytes((unsigned char*)&rnd, 4)

typedef struct {
  uint32_t shares[NUM_SHARES_M];
} mask_m_uint32_t;

typedef struct {
  uint32_t shares[NUM_SHARES_C];
} mask_c_uint32_t;

typedef struct {
  uint32_t shares[NUM_SHARES_AD];
} mask_ad_uint32_t;

typedef struct {
  uint32_t shares[NUM_SHARES_NPUB];
} mask_npub_uint32_t;

typedef struct {
  uint32_t shares[NUM_SHARES_KEY];
} mask_key_uint32_t;



void generate_shares(uint32_t* s, int num_shares, const uint8_t* data,
                     uint64_t len);

void combine_shares(uint8_t* data, uint64_t len, const uint32_t* s,
                    int num_shares);

#endif /* SHARES_H_ */
