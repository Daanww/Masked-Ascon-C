#ifndef PRINT_SIMULATION_H_
#define PRINT_SIMULATION_H_

#include <stdio.h>

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


void puthex_char(unsigned char h);
void putdec(uint32_t n);
void print_char_array_in_hex_little_endian(const char *name, uint8_t *array, int len);
void print_char_array_in_hex_with_name(const char *name, uint8_t *array, int len);
void print_share_in_hex(share_t *share);
void my_printf(const char *format, ...);
void print_state(const char *name, ascon_state_t *s);
void print_data_shares_in_hex(mask_npub_uint32_t *s, int len);
void generate_shares_with_print(uint32_t *s, int num_shares, const uint8_t *data,
                                uint64_t len);

#endif /* PRINT_SIMULATION_H_*/
