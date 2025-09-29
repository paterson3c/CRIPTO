#include "afin.h"
#include "euclides.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Encripta un archivo usando el cifrado afín.
 *
 * Fórmula de cifrado: E(x) = (a*x + b) mod m
 *
 * @param in  Archivo de entrada (texto plano). Puede ser stdin.
 * @param out Archivo de salida (texto cifrado). Puede ser stdout.
 * @param a   Clave multiplicativa (debe ser coprima con m).
 * @param b   Clave aditiva.
 * @param m   Módulo (tamaño del alfabeto).
 */
void encriptar_afin(FILE *in, FILE *out, const mpz_t a, const mpz_t b, const mpz_t m) {
    if (!in || !out) {
        fprintf(stderr, "Error: archivo de entrada o salida en NULL\n");
        return;
    }

    if (!a || !b || !m) {
        fprintf(stderr, "Error: parámetros GMP en NULL\n");
        return;
    }

    int c;
    mpz_t x, y;
    mpz_inits(x, y, NULL);

    while ((c = fgetc(in)) != EOF) {
        // Convertir carácter a número x
        mpz_set_ui(x, (unsigned char)c);

        // y = (a*x + b) mod m
        mpz_mul(y, a, x);      // y = a*x
        mpz_add(y, y, b);      // y = y + b
        mpz_mod(y, y, m);      // y = y mod m

        // Escribir resultado como byte
        fputc((unsigned char)mpz_get_ui(y), out);
    }

    mpz_clears(x, y, NULL);
}

/**
 * @brief Descifra un archivo usando el cifrado afín.
 *
 * Fórmula: D(y) = a^{-1} * (y - b) mod m
 *
 * @param in  Archivo de entrada (cifrado). Puede ser stdin.
 * @param out Archivo de salida (texto claro). Puede ser stdout.
 * @param a   Clave multiplicativa (debe ser coprima con m).
 * @param b   Clave aditiva.
 * @param m   Módulo (tamaño del alfabeto, típicamente 26 o 256).
 */
void decriptar_afin(FILE *in, FILE *out, const mpz_t a, const mpz_t b, const mpz_t m) {
    if (!in || !out) {
        fprintf(stderr, "Error: archivo de entrada o salida en NULL\n");
        return;
    }
    if (!a || !b || !m) {
        fprintf(stderr, "Error: parámetros GMP en NULL\n");
        return;
    }

    // 1) Inverso modular de a mod m usando Euclides extendido
    ExtendedEuclidesResult ext = extended_euclides(a, m);
    if (mpz_cmp_ui(ext.mcd, 1) != 0) {
        fprintf(stderr, "Error: a y m no son coprimos (mcd != 1); no existe inverso modular.\n");
        mpz_clears(ext.mcd, ext.s, ext.t, NULL);
        return;
    }
    mpz_t ainv;               // a^{-1} mod m
    mpz_init(ainv);
    mpz_mod(ainv, ext.s, m);  // ainv = s mod m (ajusta a [0, m))

    // 2) Descifrado byte a byte: x = ainv * ((y - b) mod m) mod m
    int c;
    mpz_t y, tmp, x;
    mpz_inits(y, tmp, x, NULL);

    while ((c = fgetc(in)) != EOF) {
        mpz_set_ui(y, (unsigned char)c);  // y = byte cifrado

        // tmp = (y - b) mod m   (asegurando no-negativo)
        mpz_sub(tmp, y, b);
        mpz_mod(tmp, tmp, m);

        // x = (ainv * tmp) mod m
        mpz_mul(x, ainv, tmp);
        mpz_mod(x, x, m);

        fputc((unsigned char)mpz_get_ui(x), out);
    }

    // 3) Limpieza
    mpz_clears(y, tmp, x, NULL);
    mpz_clear(ainv);
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
        encriptar_afin(in, out, a, b, m);
    } else if (mode == DECIPHER_AFIN) {
        decriptar_afin(in, out, a, b, m);
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