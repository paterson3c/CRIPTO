#include "afin.h"
#include "euclides.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Normaliza un carácter al alfabeto A–Z (m=26).
 * Convierte tildes y diéresis en su vocal base,
 * convierte ñ/Ñ en N y descarta lo que no sea A–Z.
 *
 * @param c Carácter de entrada.
 * @return Carácter normalizado en A–Z o 0 si no es válido.
 */
char normalizar_char(FILE *in) {
    int c = fgetc(in);
    if (c == EOF) return EOF;

    // ASCII simple
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    if (c >= 'A' && c <= 'Z') return c;

    // UTF-8: caracteres multibyte (empezando por 0xC3)
    if (c == 0xC3) {
        int next = fgetc(in);
        if (next == EOF) return EOF;
        switch (next) {
            // Vocales mayúsculas
            case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: return 'A'; // ÀÁÂÃÄ
            case 0x88: case 0x89: case 0x8A: case 0x8B: return 'E';             // ÈÉÊË
            case 0x8C: case 0x8D: case 0x8E: case 0x8F: return 'I';             // ÌÍÎÏ
            case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: return 'O'; // ÒÓÔÕÖ
            case 0x99: case 0x9A: case 0x9B: case 0x9C: return 'U';             // ÙÚÛÜ
            case 0x91: return 'N';                                              // Ñ

            // Vocales minúsculas
            case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: return 'A'; // àáâãä
            case 0xA8: case 0xA9: case 0xAA: case 0xAB: return 'E';             // èéêë
            case 0xAC: case 0xAD: case 0xAE: case 0xAF: return 'I';             // ìíîï
            case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: return 'O'; // òóôõö
            case 0xB9: case 0xBA: case 0xBB: case 0xBC: return 'U';             // ùúûü
            case 0xB1: return 'N';                                              // ñ
            default: return 0;
        }
    }

    // Otros caracteres no válidos
    return 0;
}

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

    EuclidesResult euc = euclides(a, m);

    if (mpz_cmp_ui(euc.rn, 1) != 0) {
        fprintf(stderr, "Error: a y M no son coprimos\n");
        exit(1);
    }

    while ((c = normalizar_char(in)) != EOF) {
        if (c == 0) 
            continue;

        // mapear A=0 ... Z=25
        int idx = c - 'A';

        // y = (a*x + b) mod 26
        mpz_set_ui(x, idx);
        mpz_mul(y, a, x);
        mpz_add(y, y, b);
        mpz_mod(y, y, m);   // m=26 fijo

        // volver a letra
        int out_c = (int)mpz_get_ui(y) + 'A';
        fputc(out_c, out);
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
        if (c < 'A' || c > 'Z') continue; // ignorar todo lo que no sea letra

        int idx = c - 'A';
        mpz_set_ui(y, idx);

        // tmp = (y - b) mod m
        mpz_sub(tmp, y, b);
        mpz_mod(tmp, tmp, m);

        // x = ainv * tmp mod m
        mpz_mul(x, ainv, tmp);
        mpz_mod(x, x, m);

        int out_c = (int)mpz_get_ui(x) + 'A';
        fputc(out_c, out);
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