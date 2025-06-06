#ifndef ASCON_H_
#define ASCON_H_

#include <stdint.h>

#include "shares.h"
#include "word.h"

typedef struct {
  word_t x[6];
} ascon_state_t;

void ascon_initaead(ascon_state_t* s, const mask_key_uint32_t* k,
                    const mask_npub_uint32_t* n);
void ascon_adata(ascon_state_t* s, const mask_ad_uint32_t* as, uint64_t alen, unsigned char** a);
void ascon_encrypt(ascon_state_t* s, mask_c_uint32_t* cs,
                             const mask_m_uint32_t* ms, uint64_t mlen, unsigned char** c, unsigned char** m);
void ascon_decrypt(ascon_state_t* s, mask_m_uint32_t* ms,
                             const mask_c_uint32_t* cs, uint64_t clen, unsigned char** m, unsigned char** c);
void ascon_final(ascon_state_t* s, const mask_key_uint32_t* k);
void ascon_settag(ascon_state_t* s, mask_c_uint32_t* cs, unsigned char* c);
void ascon_xortag(ascon_state_t* s, const mask_c_uint32_t* cs, unsigned char* c);
int ascon_iszero(ascon_state_t* s);


#endif /* ASCON_H_ */
