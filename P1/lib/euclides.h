#include <gmp.h>

/* Resultado del algoritmo de Euclides */
/**
 * @struct EuclidesResult
 * @brief Structure to store the result of the Euclidean algorithm.
 *
 * This structure contains:
 * - q: Pointer to an array of mpz_t values representing the quotients at each step.
 * - rn: The remainder (as an mpz_t) from the last step of the algorithm.
 * - n: The number of steps performed in the algorithm.
 */
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