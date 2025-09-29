#include "euclides.h"
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>

EuclidesResult euclides(const mpz_t a, const mpz_t b) {
    EuclidesResult res;
    res.q = NULL;
    res.n = 1;
    mpz_init(res.rn);

    mpz_t *r = NULL;
    r = realloc(r, 2 * sizeof(mpz_t));
    mpz_init(r[0]); mpz_set(r[0], a); // r0 = a
    mpz_init(r[1]); mpz_set(r[1], b); // r1 = b

    while (mpz_sgn(r[res.n]) != 0) {    // while rn != 0
        // ampliar para un nuevo cociente y resto
        res.q = realloc(res.q, res.n * sizeof(mpz_t));
        mpz_init(res.q[res.n - 1]); // q1..qn -> res.q[0..n-1]
        r = realloc(r, (res.n + 2) * sizeof(mpz_t));
        mpz_init(r[res.n + 1]);

        // qn <- ⌊ r_{n-1} / r_n ⌋
        mpz_fdiv_q(res.q[res.n - 1], r[res.n - 1], r[res.n]);

        // r_{n+1} <- r_{n-1} – qn * r_n
        mpz_mul(r[res.n + 1], res.q[res.n - 1], r[res.n]);
        mpz_sub(r[res.n + 1], r[res.n - 1], r[res.n + 1]);

        // n <- n + 1
        res.n++;
    }

    // n <- n - 1
    res.n--;

    // rn = último resto no nulo
    mpz_set(res.rn, r[res.n]);

    // limpiar array de restos
    for (int i = 0; i <= res.n; i++) mpz_clear(r[i]);
    free(r);

    return res;
}

/**
 * Entrada: a, b
r0 ← a
r1 ← b
s0 ← 1     ; porque 1·a + 0·b = a
s1 ← 0     ; porque 0·a + 1·b = b
t0 ← 0
t1 ← 1
n ← 1

while rn ≠ 0 do
    qn ← ⌊ r_{n-1} / r_n ⌋
    r_{n+1} ← r_{n-1} – qn · r_n
    s_{n+1} ← s_{n-1} – qn · s_n
    t_{n+1} ← t_{n-1} – qn · t_n
    n ← n + 1
end while

n ← n - 1    ; corregimos el exceso
return (q1, …, qn, rn, sn, tn)
 */
ExtendedEuclidesResult extended_euclides(const mpz_t a, const mpz_t b) {
    ExtendedEuclidesResult res;
    mpz_inits(res.mcd, res.s, res.t, NULL);

    // r0 = a, r1 = b
    mpz_t r0, r1, r2;
    mpz_inits(r0, r1, r2, NULL);
    mpz_set(r0, a);
    mpz_set(r1, b);

    // s0 = 1, s1 = 0
    mpz_t s0, s1, s2;
    mpz_inits(s0, s1, s2, NULL);
    mpz_set_ui(s0, 1);
    mpz_set_ui(s1, 0);

    // t0 = 0, t1 = 1
    mpz_t t0, t1, t2;
    mpz_inits(t0, t1, t2, NULL);
    mpz_set_ui(t0, 0);
    mpz_set_ui(t1, 1);

    // q = cociente temporal
    mpz_t q;
    mpz_init(q);

    while (mpz_sgn(r1) != 0) {
        // q = r0 / r1
        mpz_fdiv_q(q, r0, r1);

        // r2 = r0 - q*r1
        mpz_mul(r2, q, r1);
        mpz_sub(r2, r0, r2);

        // s2 = s0 - q*s1
        mpz_mul(s2, q, s1);
        mpz_sub(s2, s0, s2);

        // t2 = t0 - q*t1
        mpz_mul(t2, q, t1);
        mpz_sub(t2, t0, t2);

        // avanzar
        mpz_set(r0, r1);
        mpz_set(r1, r2);

        mpz_set(s0, s1);
        mpz_set(s1, s2);

        mpz_set(t0, t1);
        mpz_set(t1, t2);
    }

    // r0 = gcd, s0 y t0 son los coeficientes de Bézout
    mpz_set(res.mcd, r0);
    mpz_set(res.s, s0);
    mpz_set(res.t, t0);

    // limpiar temporales
    mpz_clears(r0, r1, r2, s0, s1, s2, t0, t1, t2, q, NULL);

    return res;
}

/**int main(void) {
    mpz_t a, b, lhs;
    mpz_inits(a, b, lhs, NULL);

    // Ejemplo de prueba
    mpz_set_ui(a, 5);
    mpz_set_ui(b, 26);

    ExtendedEuclidesResult res = extended_euclides(a, b);

    // 1. Mostrar resultados
    gmp_printf("a = %Zd, b = %Zd\n", a, b);
    gmp_printf("gcd = %Zd\n", res.mcd);
    gmp_printf("s = %Zd, t = %Zd\n", res.s, res.t);

    // 2. Comprobar identidad de Bézout: a*s + b*t = gcd
    mpz_mul(lhs, a, res.s);
    mpz_addmul(lhs, b, res.t);   // lhs = a*s + b*t

    gmp_printf("a*s + b*t = %Zd\n", lhs);

    if (mpz_cmp(lhs, res.mcd) == 0) {
        printf("Identidad de Bézout verificada\n");
    } else {
        printf("Error en la identidad de Bézout\n");
    }

    // Liberar
    mpz_clears(a, b, lhs, NULL);
    mpz_clears(res.mcd, res.s, res.t, NULL);

    return 0;
}**/
