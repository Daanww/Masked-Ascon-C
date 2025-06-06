
#include "shares.h"

#include <string.h>

#include "bendian.h"
#include "forceinline.h"
#include "interleave.h"

forceinline uint32_t ROR32(uint32_t x, int n) {
  return x >> n | x << (-n & 31);
}

forceinline uint64_t ROR64(uint64_t x, int n) {
  return x >> n | x << (-n & 63);
}

void generate_shares(uint32_t* s, int num_shares, const uint8_t* data,
                     uint64_t len) {
  uint32_t rnd0, rnd1;
  uint64_t rnd, i;
  /* generate random shares */
  for (i = 0; i < NUM_WORDS(len); i += 2) {
    s[(i + 0) * num_shares + 0] = 0;
    s[(i + 1) * num_shares + 0] = 0;
    for (int d = 1; d < num_shares; ++d) {
      RND(rnd0);
      RND(rnd1);
      s[(i + 0) * num_shares + d] = rnd0;
      s[(i + 1) * num_shares + d] = rnd1;
#if ASCON_EXTERN_BI
      s[(i + 0) * num_shares + 0] ^= ROR32(rnd0, ROT(d));
      s[(i + 1) * num_shares + 0] ^= ROR32(rnd1, ROT(d));
#else
      rnd = ROR64((uint64_t)rnd1 << 32 | rnd0, ROT(2 * d));
//        rnd = (uint64_t)rnd1 << 32 | rnd0;
      s[(i + 0) * num_shares + 0] ^= (uint32_t)rnd;
      s[(i + 1) * num_shares + 0] ^= (uint32_t)(rnd >> 32);
#endif
    }
  }
  /* mask complete words */
  for (i = 0; i < len / 8; ++i) {
    uint64_t x;
    memcpy(&x, data + i * 8, 8);
    x = U64BIG(x);
#if ASCON_EXTERN_BI
    x = TOBI(x);
#endif
    s[(2 * i + 0) * num_shares + 0] ^= (uint32_t)x;
    s[(2 * i + 1) * num_shares + 0] ^= (uint32_t)(x >> 32);
  }
  /* mask remaining bytes */
  if ((len / 8 * 8) != len) {
    uint64_t x = 0;
    for (i = (len / 8) * 8; i < len; ++i) {
      x ^= (uint64_t)data[i] << ((i % 8) * 8);
    }
    x = U64BIG(x);
#if ASCON_EXTERN_BI
    x = TOBI(x);
#endif
    s[(2 * (len / 8) + 0) * num_shares + 0] ^= (uint32_t)x;
    s[(2 * (len / 8) + 1) * num_shares + 0] ^= (uint32_t)(x >> 32);
  }
}

void combine_shares(uint8_t* data, uint64_t len, const uint32_t* s,
                    int num_shares) {
  uint32_t rnd0, rnd1;
  uint64_t i;
  /* unmask complete words */
  for (i = 0; i < len / 8; ++i) {
    uint64_t x = 0;
    for (int d = 0; d < num_shares; ++d) {
      rnd0 = s[(2 * i + 0) * num_shares + d];
      rnd1 = s[(2 * i + 1) * num_shares + d];
#if ASCON_EXTERN_BI
      x ^= (uint64_t)ROR32(rnd0, ROT(d));
      x ^= (uint64_t)ROR32(rnd1, ROT(d)) << 32;
#else
      x ^= ROR64((uint64_t)rnd1 << 32 | rnd0, ROT(2 * d));
//      x ^= (uint64_t)rnd1 << 32 | rnd0;
#endif
    }
#if ASCON_EXTERN_BI
    x = FROMBI(x);
#endif
    x = U64BIG(x);
    memcpy(data + i * 8, &x, 8);
  }
  /* unmask remaining bytes */
  if ((len / 8 * 8) != len) {
    uint64_t x = 0;
    for (int d = 0; d < num_shares; ++d) {
      rnd0 = s[(2 * (len / 8) + 0) * num_shares + d];
      rnd1 = s[(2 * (len / 8) + 1) * num_shares + d];
#if ASCON_EXTERN_BI
      x ^= (uint64_t)ROR32(rnd0, ROT(d));
      x ^= (uint64_t)ROR32(rnd1, ROT(d)) << 32;
#else
      x ^= ROR64((uint64_t)rnd1 << 32 | rnd0, ROT(2 * d));
//      x ^= (uint64_t)rnd1 << 32 | rnd0;
#endif
    }
#if ASCON_EXTERN_BI
    x = FROMBI(x);
#endif
    x = U64BIG(x);
    for (i = (len / 8) * 8; i < len; ++i) {
      data[i] = x >> ((i % 8) * 8);
    }
  }
}
