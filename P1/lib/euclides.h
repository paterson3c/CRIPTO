#include <gmp.h>

/* Resultado del algoritmo de Euclides */
typedef struct _EuclidesResult {
    mpz_t *q;
    mpz_t rn;
    int n;
} EuclidesResult;

typedef struct _ExtendedEuclidesResult {
    mpz_t s;   /*inverso a*/
    mpz_t t;   /*inverso b*/
    mpz_t mcd; /*mcd*/
} ExtendedEuclidesResult;

EuclidesResult euclides(const mpz_t a, const mpz_t b);
ExtendedEuclidesResult extended_euclides(const mpz_t a, const mpz_t b);