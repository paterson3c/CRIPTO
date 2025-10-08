#include "afin.h"
#include "euclides.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 26  // tamaño de bloque k fijo
#define SEED 9391239

/**
 * @brief Lee un carácter UTF-8 y lo normaliza a A–Z.
 */
int normalizar_char(FILE *in) {
    int c = fgetc(in);
    if (c == EOF) return EOF;

    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    if (c >= 'A' && c <= 'Z') return c;

    if (c == 0xC3) {
        int next = fgetc(in);
        if (next == EOF) return EOF;
        switch (next) {
            // Vocales mayúsculas
            case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: return 'A';
            case 0x88: case 0x89: case 0x8A: case 0x8B: return 'E';
            case 0x8C: case 0x8D: case 0x8E: case 0x8F: return 'I';
            case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: return 'O';
            case 0x99: case 0x9A: case 0x9B: case 0x9C: return 'U';
            case 0x91: return 'N';
            // Vocales minúsculas
            case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: return 'A';
            case 0xA8: case 0xA9: case 0xAA: case 0xAB: return 'E';
            case 0xAC: case 0xAD: case 0xAE: case 0xAF: return 'I';
            case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: return 'O';
            case 0xB9: case 0xBA: case 0xBB: case 0xBC: return 'U';
            case 0xB1: return 'N';
            default: return 0;
        }
    }
    return 0;
}

/**
 * @brief Genera una permutación pseudoaleatoria del bloque.
 * @param perm Array con la permutación (salida).
 * @param size Tamaño del bloque.
 * @param seed Semilla para el generador.
 */
void generar_permutacion(int *perm, int size, unsigned int seed) {
    for (int i = 0; i < size; i++) perm[i] = i;
    srand(seed);
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
    }
}

/**
 * @brief Cifra un archivo por bloques con permutación.
 * Aplica el cifrado afín dentro de cada bloque y permuta las posiciones.
 */
void encriptar_afin_modificado(FILE *in, FILE *out, const mpz_t a, const mpz_t b, const mpz_t m) {
    if (!in || !out) return;

    int c, count = 0;
    char bloque[BLOCK_SIZE];
    int perm[BLOCK_SIZE];
    generar_permutacion(perm, BLOCK_SIZE, SEED);

    mpz_t x, y;
    mpz_inits(x, y, NULL);

    while ((c = normalizar_char(in)) != EOF) {
        if (c == 0) continue;
        bloque[count++] = c;
        if (count == BLOCK_SIZE) {
            // cifrar bloque completo
            char cifrado[BLOCK_SIZE];
            for (int i = 0; i < BLOCK_SIZE; i++) {
                int idx = bloque[i] - 'A';
                mpz_set_ui(x, idx);
                mpz_mul(y, a, x);
                mpz_add(y, y, b);
                mpz_mod(y, y, m);
                int enc = (int)mpz_get_ui(y) + 'A';
                cifrado[perm[i]] = enc;
            }
            fwrite(cifrado, sizeof(char), BLOCK_SIZE, out);
            count = 0;
        }
    }

    // procesar bloque incompleto
    if (count > 0) {
        char cifrado[BLOCK_SIZE];
        for (int i = 0; i < count; i++) {
            int idx = bloque[i] - 'A';
            mpz_set_ui(x, idx);
            mpz_mul(y, a, x);
            mpz_add(y, y, b);
            mpz_mod(y, y, m);
            int enc = (int)mpz_get_ui(y) + 'A';
            cifrado[perm[i]] = enc;
        }
        fwrite(cifrado, sizeof(char), count, out);
    }

    mpz_clears(x, y, NULL);
}

/**
 * @brief Descifra el archivo cifrado con afin_modificado.
 * Aplica la permutación inversa y el descifrado afín clásico.
 */
void decriptar_afin_modificado(FILE *in, FILE *out, const mpz_t a, const mpz_t b, const mpz_t m) {
    if (!in || !out) return;

    ExtendedEuclidesResult ext = extended_euclides(a, m);
    if (mpz_cmp_ui(ext.mcd, 1) != 0) {
        fprintf(stderr, "No existe inverso de a mod m.\n");
        return;
    }

    mpz_t ainv, y, tmp, x;
    mpz_inits(ainv, y, tmp, x, NULL);
    mpz_mod(ainv, ext.s, m);

    int perm[BLOCK_SIZE], invperm[BLOCK_SIZE];
    generar_permutacion(perm, BLOCK_SIZE, SEED);
    for (int i = 0; i < BLOCK_SIZE; i++) invperm[perm[i]] = i;

    char bloque[BLOCK_SIZE], decif[BLOCK_SIZE];
    size_t n;

    while ((n = fread(bloque, sizeof(char), BLOCK_SIZE, in)) > 0) {
        for (size_t i = 0; i < n; i++) {
            int idx = bloque[i] - 'A';
            mpz_set_ui(y, idx);
            mpz_sub(tmp, y, b);
            mpz_mod(tmp, tmp, m);
            mpz_mul(x, ainv, tmp);
            mpz_mod(x, x, m);
            int dec = (int)mpz_get_ui(x) + 'A';
            decif[invperm[i]] = dec;
        }
        fwrite(decif, sizeof(char), n, out);
    }

    mpz_clears(ainv, y, tmp, x, NULL);
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