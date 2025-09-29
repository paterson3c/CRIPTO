#include "afin.h"
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>

void encrypt(int m, int a, int b, FILE *in, FILE *out) {

}
void decrypt(int m, int a, int b, FILE *in, FILE *out);

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

    int m = 0, a = 0, b = 0, i = 0;
    int mode = -1;
    const char *input_path = NULL;
    const char *output_path = NULL;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-C") == 0) {
            mode = CIPHER_AFIN;
        } else if (strcmp(argv[i], "-D") == 0) {
            mode = DECIPHER_AFIN;
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            m = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            a = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            b = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_path = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else {
            fprintf(stderr, "Argumento no reconocido: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

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

    if (mode == 1) {
        encrypt(m, a, b, in, out);
    } else if (mode == 0) {
        decrypt(m, a, b, in, out);
    } else {
        fprintf(stderr, "Debes especificar -C (cifrar) o -D (descifrar).\n");
        return EXIT_FAILURE;
    }

    if (in != stdin) fclose(in);
    if (out != stdout) fclose(out);

    return EXIT_SUCCESS;
}