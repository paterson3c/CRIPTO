#include "euclides.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#define BLOCK_SIZE 26

/* ---------- Conversiones base-26 ---------- */

void block_to_mpz(const char *block, int L, mpz_t x) {
    mpz_set_ui(x, 0);
    for (int i = 0; i < L; ++i) {
        int d = block[i] - 'A';
        if (d < 0 || d > 25) d = 0; // saneo
        mpz_mul_ui(x, x, 26);
        mpz_add_ui(x, x, (unsigned)d);
    }
}

void mpz_to_block(const mpz_t x_in, int L, char *block_out) {
    mpz_t x, q, r;
    mpz_inits(x, q, r, NULL);
    mpz_set(x, x_in);

    for (int i = 0; i < L; ++i) block_out[i] = 'A'; // padding por defecto

    for (int pos = L - 1; pos >= 0 && mpz_sgn(x) > 0; --pos) {
        mpz_tdiv_qr_ui(q, r, x, 26);
        unsigned long rem = mpz_get_ui(r);
        block_out[pos] = (char)('A' + rem);
        mpz_set(x, q);
    }

    mpz_clears(x, q, r, NULL);
}

void compute_modulus(int L, mpz_t M) {
    mpz_ui_pow_ui(M, 26, (unsigned long)L);
}

/* ---------- Cifrar / Descifrar por bloques ---------- */

void encriptar_afin_bloques(FILE *in, FILE *out,
                            const mpz_t a, const mpz_t b, const mpz_t M) {
    char bloque[BLOCK_SIZE];
    size_t count = 0;
    mpz_t x, y;
    mpz_inits(x, y, NULL);

    ExtendedEuclidesResult ext = extended_euclides(a, M);
    if (mpz_cmp_ui(ext.mcd, 1) != 0) {
        fprintf(stderr, "No existe inverso de a mod M.\n");
        mpz_clears(ext.mcd, ext.s, ext.t, NULL);
        return;
    }

    int c;
    while ((c = fgetc(in)) != EOF) {
        if (c >= 'a' && c <= 'z') c -= 32; // minúscula → mayúscula
        if (c < 'A' || c > 'Z') continue;  // ignora no letras
        bloque[count++] = (char)c;

        if (count == BLOCK_SIZE) {
            block_to_mpz(bloque, BLOCK_SIZE, x);
            mpz_mul(y, a, x);
            mpz_add(y, y, b);
            mpz_mod(y, y, M);
            mpz_to_block(y, BLOCK_SIZE, bloque);
            fwrite(bloque, sizeof(char), BLOCK_SIZE, out);
            count = 0;
        }
    }

    // último bloque (rellenar con 'A')
    if (count > 0) {
        for (size_t i = count; i < BLOCK_SIZE; ++i)
            bloque[i] = 'A';
        block_to_mpz(bloque, BLOCK_SIZE, x);
        mpz_mul(y, a, x);
        mpz_add(y, y, b);
        mpz_mod(y, y, M);
        mpz_to_block(y, BLOCK_SIZE, bloque);
        fwrite(bloque, sizeof(char), BLOCK_SIZE, out);
    }

    mpz_clears(x, y, NULL);
}

void decriptar_afin_bloques(FILE *in, FILE *out,
                            const mpz_t a, const mpz_t b, const mpz_t M) {
    ExtendedEuclidesResult ext = extended_euclides(a, M);
    if (mpz_cmp_ui(ext.mcd, 1) != 0) {
        fprintf(stderr, "No existe inverso de a mod M.\n");
        mpz_clears(ext.mcd, ext.s, ext.t, NULL);
        return;
    }

    mpz_t a_inv, x, y, tmp;
    mpz_inits(a_inv, x, y, tmp, NULL);
    mpz_mod(a_inv, ext.s, M);

    char bloque[BLOCK_SIZE];
    while (fread(bloque, sizeof(char), BLOCK_SIZE, in) == BLOCK_SIZE) {
        block_to_mpz(bloque, BLOCK_SIZE, y);
        mpz_sub(tmp, y, b);
        mpz_mod(tmp, tmp, M);
        mpz_mul(x, a_inv, tmp);
        mpz_mod(x, x, M);
        mpz_to_block(x, BLOCK_SIZE, bloque);
        fwrite(bloque, sizeof(char), BLOCK_SIZE, out);
    }

    mpz_clears(a_inv, x, y, tmp, NULL);
    mpz_clears(ext.mcd, ext.s, ext.t, NULL);
}

/* ---------- Programa principal ---------- */

int main(int argc, char *argv[]) {
    if (argc < 8) {
        fprintf(stderr, "Uso: %s -C|-D -a <clave_mult> -b <clave_add> [-i in] [-o out]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int mode = -1;
    const char *input_path = NULL, *output_path = NULL;
    char *a_str = NULL, *b_str = NULL;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-C")) mode = 0;
        else if (!strcmp(argv[i], "-D")) mode = 1;
        else if (!strcmp(argv[i], "-a") && i + 1 < argc) a_str = argv[++i];
        else if (!strcmp(argv[i], "-b") && i + 1 < argc) b_str = argv[++i];
        else if (!strcmp(argv[i], "-i") && i + 1 < argc) input_path = argv[++i];
        else if (!strcmp(argv[i], "-o") && i + 1 < argc) output_path = argv[++i];
    }

    FILE *in = input_path ? fopen(input_path, "r") : stdin;
    FILE *out = output_path ? fopen(output_path, "w") : stdout;
    if (!in || !out) { perror("fopen"); return EXIT_FAILURE; }

    mpz_t a, b, M;
    mpz_inits(a, b, M, NULL);
    mpz_set_str(a, a_str, 10);
    mpz_set_str(b, b_str, 10);
    compute_modulus(BLOCK_SIZE, M);

    if (mode == 0)
        encriptar_afin_bloques(in, out, a, b, M);
    else if (mode == 1)
        decriptar_afin_bloques(in, out, a, b, M);
    else
        fprintf(stderr, "Debes indicar -C o -D.\n");

    mpz_clears(a, b, M, NULL);
    if (in != stdin) fclose(in);
    if (out != stdout) fclose(out);
    return EXIT_SUCCESS;
}
