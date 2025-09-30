#include <stdio.h>
#include <gmp.h>

#define CIPHER_AFIN 1
#define DECIPHER_AFIN 0

void encriptar_afin_modificado(FILE *in, FILE *out, const mpz_t a, const mpz_t b, const mpz_t m);
void decriptar_afin_modificado(FILE *in, FILE *out, const mpz_t a, const mpz_t b, const mpz_t m);