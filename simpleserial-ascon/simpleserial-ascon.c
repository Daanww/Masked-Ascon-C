/*
    This file is part of the ChipWhisperer Example Targets
    Copyright (C) 2012-2017 NewAE Technology Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "crypto_aead.h"
//#include "aes-independant.h"
#include "simpleserial.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "api.h"

uint8_t key[CRYPTO_KEYBYTES] = {0};
uint8_t nonce[CRYPTO_NBYTES] = {0};
uint8_t ad[CRYPTO_ABYTES] = {0};



//uint8_t get_mask(uint8_t* m, uint8_t len)
//{
//  aes_indep_mask(m, len);
//  return 0x00;
//}

uint8_t get_key(uint8_t* k, uint8_t len)
{
    memcpy(key, k, CRYPTO_KEYBYTES);
	return 0x00;
}

uint8_t get_nonce(uint8_t* n, uint8_t len)
{
    memcpy(nonce, n, CRYPTO_NBYTES);
    return 0x00;
}

uint8_t get_associated_data(uint8_t* a, uint8_t len)
{
    memcpy(ad, a, CRYPTO_ABYTES);
    return 0x00;
}

uint8_t get_seed(uint8_t* s, uint8_t len) {
    //set the seed
    if (len < 4) {
        // If the length of s is less than 4, we can't form a 32-bit seed
        return 0x00;
    }

    // Combine the first 4 bytes into a 32-bit seed
    uint32_t seed = 0;
    for (int i = 0; i < 4; i++) {
        seed |= ((uint32_t)s[i] << (8 * i));
    }

    // Seed the random number generator with the 32-bit seed
    srand(seed);
    return 0x00;
}

/*uint8_t get_pt(uint8_t* pt, uint8_t len)
{
    aes_indep_enc_pretrigger(pt);

	trigger_high();

  #ifdef ADD_JITTER
  for (volatile uint8_t k = 0; k < (*pt & 0x0F); k++);
  #endif

	aes_indep_enc(pt); *//* encrypting the data block *//*
	trigger_low();

    aes_indep_enc_posttrigger(pt);

	simpleserial_put('r', 16, pt);
	return 0x00;
}*/

uint8_t get_pt(uint8_t* pt, uint8_t len)
{
//    aes_indep_enc_pretrigger(pt);
//
    trigger_high();
//
//#ifdef ADD_JITTER
//    for (volatile uint8_t k = 0; k < (*pt & 0x0F); k++);
//#endif

//    unsigned char assoc_data[16] = "I_AM_ASSOC_DATA";
//    unsigned char nonce[16] = "I_AM_NONCE_:)__";
//    unsigned char key[16] = "THIS_IS_THE_KEY";
    crypto_aead_encrypt(pt, (unsigned long long*)&len, pt, len, ad, 16, nonce, key);


//    aes_indep_enc(pt); /* encrypting the data block */
    trigger_low();

//    aes_indep_enc_posttrigger(pt);

    simpleserial_put('r', 16, pt);
    return 0x00;
}

//uint8_t reset(uint8_t* x, uint8_t len)
//{
//    // Reset key here if needed
//	return 0x00;
//}

//static uint16_t num_encryption_rounds = 10;

//uint8_t enc_multi_getpt(uint8_t* pt, uint8_t len)
//{
//    aes_indep_enc_pretrigger(pt);
//
//    for(unsigned int i = 0; i < num_encryption_rounds; i++){
//        trigger_high();
//        aes_indep_enc(pt);
//        trigger_low();
//    }
//
//    aes_indep_enc_posttrigger(pt);
//	simpleserial_put('r', 16, pt);
//    return 0;
//}
//
//uint8_t enc_multi_setnum(uint8_t* t, uint8_t len)
//{
//    //Assumes user entered a number like [0, 200] to mean "200"
//    //which is most sane looking for humans I think
//    num_encryption_rounds = t[1];
//    num_encryption_rounds |= t[0] << 8;
//    return 0;
//}

uint8_t info(uint8_t* x, uint8_t len)
{
    print("Simpleserial-ascon on Ibex, compiled ");
    print(__DATE__);
    print(", ");
    print(__TIME__);
    print("\n");
    return 0x00;
}

//#if SS_VER == SS_VER_2_1
//uint8_t aes(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
//{
//    uint8_t req_len = 0;
//    uint8_t err = 0;
//    uint8_t mask_len = 0;
//    if (scmd & 0x04) {
//        // Mask has variable length. First byte encodes the length
//        mask_len = buf[req_len];
//        req_len += 1 + mask_len;
//        if (req_len > len) {
//            return SS_ERR_LEN;
//        }
//        err = get_mask(buf + req_len - mask_len, mask_len);
//        if (err)
//            return err;
//    }
//
//    if (scmd & 0x02) {
//        req_len += 16;
//        if (req_len > len) {
//            return SS_ERR_LEN;
//        }
//        err = get_key(buf + req_len - 16, 16);
//        if (err)
//            return err;
//    }
//    if (scmd & 0x01) {
//        req_len += 16;
//        if (req_len > len) {
//            return SS_ERR_LEN;
//        }
//        err = get_pt(buf + req_len - 16, 16);
//        if (err)
//            return err;
//    }
//
//    if (len != req_len) {
//        return SS_ERR_LEN;
//    }
//
//    return 0x00;
//
//}
//#endif

int main(void)
{
//	uint8_t tmp[KEY_LENGTH] = {DEFAULT_KEY};

    platform_init();
    init_uart();
    trigger_setup();

//	aes_indep_init();
//	aes_indep_key(tmp);

    /* Uncomment this to get a HELLO message for debug */

    // putch('h');
    // putch('e');
    // putch('l');
    // putch('l');
    // putch('o');
    // putch('\n');

	simpleserial_init();
//    #if SS_VER == SS_VER_2_1
//    simpleserial_addcmd(0x01, 16, aes);
//    #else
    simpleserial_addcmd('k', CRYPTO_KEYBYTES, get_key);
    simpleserial_addcmd('p', 16,  get_pt);
    simpleserial_addcmd('n', CRYPTO_NBYTES, get_nonce);
    simpleserial_addcmd('a', CRYPTO_ABYTES, get_associated_data);
    simpleserial_addcmd('s', 4, get_seed);
//    simpleserial_addcmd('x',  0,   reset);
//    simpleserial_addcmd_flags('m', 18, get_mask, CMD_FLAG_LEN);
//    simpleserial_addcmd('s', 2, enc_multi_setnum);
//    simpleserial_addcmd('f', 16, enc_multi_getpt);
    simpleserial_addcmd('i', 1, info);
//    #endif
    while(1)
        simpleserial_get();
}
