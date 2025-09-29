#include <gmp.h>

typedef struct _EuclidesResult EuclidesResult;
typedef struct _ExtendedEuclidesResult ExtendedEuclidesResult;

EuclidesResult euclides(const mpz_t a, const mpz_t b);
ExtendedEuclidesResult extended_euclides(const mpz_t a, const mpz_t b);