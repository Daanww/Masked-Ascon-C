#ifndef CRYPTO_AEAD_H_
#define CRYPTO_AEAD_H_

void crypto_aead_set_key(const unsigned char* k);

int crypto_aead_encrypt(unsigned char *c, unsigned long long *clen,
                        unsigned char *m, unsigned long long mlen,
                        unsigned char *a, unsigned long long alen,
                        const unsigned char *n,
                        const unsigned char *k);


int crypto_aead_decrypt(unsigned char *m, unsigned long long *mlen,
                        unsigned char *c, unsigned long long clen,
                        unsigned char *a, unsigned long long alen,
                        const unsigned char *n,
                        const unsigned char *k);

#endif /* CRYPTO_AEAD_H_ */