#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_TEXT 1000000
#define ALPHABET 26
#define A 'A'
#define TAM 500

// Función para limpiar el texto (solo A-Z)
int load_text(const char *filename, char *buffer) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("Error abriendo fichero"); exit(EXIT_FAILURE); }
    int len = 0, c;
    while ((c = fgetc(f)) != EOF) {
        if (isalpha(c)) buffer[len++] = toupper(c);
    }
    buffer[len] = '\0';
    fclose(f);
    return len;
}

/*Calcula el maximo común divisor de 2 números*/
int mcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/*Calcula el maximo común divisor de un array de números*/
int mcd_array(int *arr, int n) {
    if (n == 0) return 0;        // sin elementos
    int resultado = arr[0];      // empieza con el primero
    for (int i = 1; i < n; i++) {
        resultado = mcd(resultado, arr[i]);
        if (resultado == 1) {    // ya no puede ser más pequeño
            break;
        }
    }
    return resultado;
}

// Test de Kasiski: busca repeticiones de trigramas y distancias
void kasiski(const char *text, int len) {
    printf("=== Test de Kasiski ===\n");
    int mcds[TAM];
    int num_mcd=0;
    for (int i = 0; i < len - 3; i++) {
        char trig[4];
        strncpy(trig, &text[i], 3);
        trig[3] = '\0';
        int num =0;
        int distancias[TAM]; 
        for (int j = i + 3; j < len - 3; j++) {
            
            if (strncmp(trig, &text[j], 3) == 0) {
                printf("Repetición %s en %d y %d (distancia %d)\n", trig, i, j, j - i);
                distancias[num] = j-i;
                num++;
            }

            if(num>=2 && j>= len -4){
                int mcd = mcd_array(distancias, num);
                printf("La cadena %s se repite %d veces. MCD calculado: %d\n", trig, num, mcd);
                mcds[num_mcd] = mcd;
                num_mcd++;

            }
        }
    }

    int tam_clave = mcd_array(mcds, num_mcd);
    printf("Tamaño de la clave: %d \n", tam_clave);
}

// Índice de coincidencia para longitud n
void ic(const char *text, int len, int n) {
    printf("=== Índice de Coincidencia para n=%d ===\n", n);
    for (int k = 0; k < n; k++) {
        int freq[ALPHABET] = {0};
        int count = 0;
        for (int i = k; i < len; i += n) {
            freq[text[i] - A]++;
            count++;
        }
        double ic_val = 0.0;
        for (int j = 0; j < ALPHABET; j++) {
            ic_val += freq[j] * (freq[j] - 1);
        }
        ic_val /= (double)(count * (count - 1));
        printf("Subcadena %d: IC = %.3f\n", k + 1, ic_val);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s {-kasiski | -ic N} -i filein\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filein = NULL;
    int n = 0;
    int mode = 0; // 1=kasiski, 2=ic

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-kasiski") == 0) mode = 1;
        else if (strcmp(argv[i], "-ic") == 0 && i + 1 < argc) { mode = 2; n = atoi(argv[++i]); }
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) filein = argv[++i];
    }

    if (!filein || mode == 0) {
        fprintf(stderr, "Parámetros incorrectos. Uso: %s {-kasiski | -ic N} -i filein\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *text = malloc(MAX_TEXT);
    int len = load_text(filein, text);

    if (mode == 1) kasiski(text, len);
    else if (mode == 2) ic(text, len, n);

    free(text);
    return 0;
}
