#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ALPHABET_SIZE 26
#define A 'A'

void vigenere(char *text, const char *key, int encrypt) {
    int klen = strlen(key);
    int j = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        char c = toupper((unsigned char)text[i]);
        if (c >= 'A' && c <= 'Z') {
            int pi = c - A;
            int ki = toupper((unsigned char)key[j % klen]) - A;
            int ci;
            if (encrypt) {
                ci = (pi + ki) % ALPHABET_SIZE;
            } else {
                ci = (pi - ki + ALPHABET_SIZE) % ALPHABET_SIZE;
            }
            text[i] = (char)(ci + A);
            j++;
        }
    }
}

int main(int argc, char *argv[]) {
    int encrypt = -1;
    char *key = NULL, *fin = NULL, *fout = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-C") == 0) {
            encrypt = 1;
        } else if (strcmp(argv[i], "-D") == 0) {
            encrypt = 0;
        } else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            key = argv[++i];
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            fin = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            fout = argv[++i];
        } else {
            fprintf(stderr, "Uso: %s {-C|-D} -k clave -i filein -o fileout\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (encrypt == -1 || key == NULL) {
        fprintf(stderr, "Debes indicar {-C|-D} y la clave con -k\n");
        return EXIT_FAILURE;
    }

    FILE *in = stdin, *out = stdout;
    if (fin) {
        in = fopen(fin, "r");
        if (!in) { perror("Error abriendo input"); return EXIT_FAILURE; }
    }
    if (fout) {
        out = fopen(fout, "w");
        if (!out) { perror("Error abriendo output"); return EXIT_FAILURE; }
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), in)) {
        vigenere(buffer, key, encrypt);
        fputs(buffer, out);
    }

    if (in != stdin) fclose(in);
    if (out != stdout) fclose(out);
    return 0;
}
