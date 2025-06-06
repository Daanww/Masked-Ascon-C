#ifndef INTERLEAVE_H_
#define INTERLEAVE_H_

#include <stdint.h>

#include "config.h"
#include "forceinline.h"

#define BD(e, o, lo, hi)                              \
  do {                                                \
    uint64_t tmp = TOBI((uint64_t)(hi) << 32 | (lo)); \
    e = (uint32_t)tmp;                                \
    o = tmp >> 32;                                    \
  } while (0)

#define BI(lo, hi, e, o)                              \
  do {                                                \
    uint64_t tmp = FROMBI((uint64_t)(o) << 32 | (e)); \
    lo = (uint32_t)tmp;                               \
    hi = tmp >> 32;                                   \
  } while (0)

uint64_t TOBI(uint64_t in);
uint64_t FROMBI(uint64_t in);

extern const uint32_t B[3];

forceinline uint32_t deinterleave32(uint32_t x) {
  uint32_t t;
  t = (x ^ (x >> 1)) & B[0], x ^= t ^ (t << 1);
  t = (x ^ (x >> 2)) & B[1], x ^= t ^ (t << 2);
  t = (x ^ (x >> 4)) & B[2], x ^= t ^ (t << 4);
  t = (x ^ (x >> 8)) & 0xff00, x ^= t ^ (t << 8);
  return x;
}

// This is 32 bit outer interleave as showing in Hacker's Delight as credited below.
forceinline uint32_t interleave32(uint32_t x) {
  uint32_t t;
  t = (x ^ (x >> 8)) & 0xff00, x ^= t ^ (t << 8);
  t = (x ^ (x >> 4)) & B[2], x ^= t ^ (t << 4);
  t = (x ^ (x >> 2)) & B[1], x ^= t ^ (t << 2);
  t = (x ^ (x >> 1)) & B[0], x ^= t ^ (t << 1);
  return x;
}

/* credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
forceinline uint64_t deinterleave64(uint64_t in) {
    // input: abcd efgh ijkl mnop qrst uvwx yz12 3456 | ABCD EFGH IJKL MNOP QRST UVWX YZ!@ #$%^
  uint32_t hi = in >> 32; // hi = abcd efgh ijkl mnop qrst uvwx yz12 3456
  uint32_t lo = in; // lo = ABCD EFGH IJKL MNOP QRST UVWX YZ!@ #$%^
  uint32_t t0, t1, e, o;
  t0 = deinterleave32(lo); // t0 =
  t1 = deinterleave32(hi);
  e = (t1 << 16) | (t0 & 0x0000FFFF);
  o = (t1 & 0xFFFF0000) | (t0 >> 16);
  return (uint64_t)o << 32 | e;
}

/* credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
forceinline uint64_t interleave64(uint64_t in) {
    // input: abcd efgh ijkl mnop qrst uvwx yz12 3456 | ABCD EFGH IJKL MNOP QRST UVWX YZ!@ #$%^
  uint32_t e = in;  // e = ABCD EFGH IJKL MNOP QRST UVWX YZ!@ #$%^
  uint32_t o = in >> 32; // o = abcd efgh ijkl mnop qrst uvwx yz12 3456
  uint32_t t0, t1, lo, hi;
  t0 = (o << 16) | (e & 0x0000FFFF); // t0 = qrst uvwx yz12 3456 QRST UVWX YZ!@ #$%^
  t1 = (o & 0xFFFF0000) | (e >> 16); // t1 = abcd efgh ijkl mnop ABCD EFGH IJKL MNOP
  lo = interleave32(t0); // lo = qQrR sS tT uUvV wWxX yYzZ 1!2@ 3#4$ 5%6^
  hi = interleave32(t1); // hi = aAbB cCdD eEfF gGhH iIjJ kKlL mMnN oOpP
  return (uint64_t)hi << 32 | lo; // output = aAbB cCdD eEfF gGhH iIjJ kKlL mMnN oOpP | qQrR sStT uUvV wWxX yYzZ 1!2@ 3#4$ 5%6^
}

#endif /* INTERLEAVE_H_ */
