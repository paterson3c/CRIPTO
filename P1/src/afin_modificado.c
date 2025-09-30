#include "afin_modificado.h"
#include "euclides.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief Encripta un archivo usando el cifrado afín por bloques de 2 bytes + permutación.
 *
 * Fórmula de cifrado: E(x) = (a*x + b) mod 65536
 *
 * @param in   Archivo de entrada.
 * @param out  Archivo de salida.
 * @param a    Clave multiplicativa (impar, coprima con 65536).
 * @param b    Clave aditiva.
 * @param seed Semilla para permutación (32 bits).
 */
void encriptar_afin_modificado(FILE *in, FILE *out, const mpz_t a, const mpz_t b, unsigned int seed) {
    const int WINDOW = 8; // nº de bloques por ventana
    uint16_t blocks[WINDOW];
    int count;
    mpz_t x, y, m;
    mpz_inits(x, y, m, NULL);
    mpz_set_ui(m, 65536);

    while (1) {
        // Leer hasta WINDOW bloques de 2 bytes
        count = 0;
        for (int i = 0; i < WINDOW; i++) {
            int c1 = fgetc(in);
            int c2 = fgetc(in);
            if (c1 == EOF) break;
            if (c2 == EOF) c2 = 0x00; // padding
            blocks[i] = ((uint16_t)c1 << 8) | (uint16_t)c2;
            count++;
        }
        if (count == 0) break;

        // Cifrar cada bloque
        for (int i = 0; i < count; i++) {
            mpz_set_ui(x, blocks[i]);
            mpz_mul(y, a, x);
            mpz_add(y, y, b);
            mpz_mod(y, y, m);
            blocks[i] = (uint16_t)mpz_get_ui(y);
        }

        // Permutar (Fisher-Yates con LCG)
        unsigned int state = seed;
        for (int i = count - 1; i > 0; i--) {
            state = state * 1664525u + 1013904223u;
            int j = state % (i + 1);
            uint16_t tmp = blocks[i];
            blocks[i] = blocks[j];
            blocks[j] = tmp;
        }

        // Escribir bloques cifrados
        for (int i = 0; i < count; i++) {
            fputc((blocks[i] >> 8) & 0xFF, out);
            fputc(blocks[i] & 0xFF, out);
        }
    }

    mpz_clears(x, y, m, NULL);
}

/**
 * @brief Descifra un archivo usando el cifrado afín por bloques de 2 bytes + permutación.
 *
 * Fórmula de descifrado: D(y) = a^{-1} * (y - b) mod 65536
 *
 * @param in   Archivo de entrada.
 * @param out  Archivo de salida.
 * @param a    Clave multiplicativa (impar, coprima con 65536).
 * @param b    Clave aditiva.
 * @param seed Semilla para permutación (32 bits).
 */
void decriptar_afin_modificado(FILE *in, FILE *out, const mpz_t a, const mpz_t b, unsigned int seed) {
    const int WINDOW = 8;
    uint16_t blocks[WINDOW];
    int count;
    mpz_t x, y, m, ainv, tmp;
    mpz_inits(x, y, m, ainv, tmp, NULL);
    mpz_set_ui(m, 65536);

    // Calcular inverso de a mod 65536
    ExtendedEuclidesResult ext = extended_euclides(a, m);
    if (mpz_cmp_ui(ext.mcd, 1) != 0) {
        fprintf(stderr, "Error: a y m no son coprimos.\n");
        mpz_clears(ext.mcd, ext.s, ext.t, NULL);
        mpz_clears(x, y, m, ainv, tmp, NULL);
        return;
    }
    mpz_mod(ainv, ext.s, m);

    while (1) {
        // Leer hasta WINDOW bloques
        count = 0;
        for (int i = 0; i < WINDOW; i++) {
            int c1 = fgetc(in);
            int c2 = fgetc(in);
            if (c1 == EOF) break;
            if (c2 == EOF) c2 = 0x00;
            blocks[i] = ((uint16_t)c1 << 8) | (uint16_t)c2;
            count++;
        }
        if (count == 0) break;

        // Regenerar la permutación y guardarla
        unsigned int state = seed;
        int perm[WINDOW];
        for (int i = 0; i < count; i++) perm[i] = i;
        for (int i = count - 1; i > 0; i--) {
            state = state * 1664525u + 1013904223u;
            int j = state % (i + 1);
            int tmpi = perm[i];
            perm[i] = perm[j];
            perm[j] = tmpi;
        }
        // Inversa de la permutación
        uint16_t inv_blocks[WINDOW];
        for (int i = 0; i < count; i++) {
            inv_blocks[perm[i]] = blocks[i];
        }
        memcpy(blocks, inv_blocks, count * sizeof(uint16_t));

        // Descifrar cada bloque
        for (int i = 0; i < count; i++) {
            mpz_set_ui(y, blocks[i]);
            mpz_sub(tmp, y, b);
            mpz_mod(tmp, tmp, m);
            mpz_mul(x, ainv, tmp);
            mpz_mod(x, x, m);
            blocks[i] = (uint16_t)mpz_get_ui(x);
        }

        // Escribir bloques descifrados
        for (int i = 0; i < count; i++) {
            fputc((blocks[i] >> 8) & 0xFF, out);
            fputc(blocks[i] & 0xFF, out);
        }
    }

    mpz_clears(x, y, m, ainv, tmp, NULL);
    mpz_clears(ext.mcd, ext.s, ext.t, NULL);
}


/**
 * @brief Main function to test the encryption and decryption functions.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Exit status.
 * -C: encrypt
 * -D: decrypt
 * -m: modulo
 * -a: multiplicative key
 * -b: additive key
 * -i: input file (default: stdin)
 * -o: output file (default: stdout)
 */
int main(int argc, char *argv[]) {
    if (argc < 8) {  
        fprintf(stderr, "Uso: %s -C|-D -m <modulo> -a <clave_mult> -b <clave_add> [-i <input>] [-o <output>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int int_m = 0, int_a = 0, int_b = 0;
    int mode = -1;
    const char *input_path = NULL;
    const char *output_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-C") == 0) {
            mode = CIPHER_AFIN;
        } else if (strcmp(argv[i], "-D") == 0) {
            mode = DECIPHER_AFIN;
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            int_m = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            int_a = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            int_b = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_path = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else {
            fprintf(stderr, "Argumento no reconocido: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    // Inicializar GMP
    mpz_t m, a, b;
    mpz_inits(m, a, b, NULL);

    // Asignar valores de los int a mpz_t
    mpz_set_ui(m, int_m);
    mpz_set_ui(a, int_a);
    mpz_set_ui(b, int_b);

    // Abrir ficheros
    FILE *in = stdin;
    FILE *out = stdout;
    if (input_path) {
        in = fopen(input_path, "r");
        if (!in) {
            perror("Error abriendo input");
            return EXIT_FAILURE;
        }
    }
    if (output_path) {
        out = fopen(output_path, "w");
        if (!out) {
            perror("Error abriendo output");
            if (in != stdin) fclose(in);
            return EXIT_FAILURE;
        }
    }

    // Ejecutar cifrado/descifrado
    if (mode == CIPHER_AFIN) {
        encriptar_afin_modificado(in, out, a, b, m);
    } else if (mode == DECIPHER_AFIN) {
        decriptar_afin_modificado(in, out, a, b, m);
    } else {
        fprintf(stderr, "Debes especificar -C (cifrar) o -D (descifrar).\n");
        return EXIT_FAILURE;
    }

    // Cerrar ficheros
    if (in != stdin) fclose(in);
    if (out != stdout) fclose(out);

    // Liberar GMP
    mpz_clears(m, a, b, NULL);

    return EXIT_SUCCESS;
}