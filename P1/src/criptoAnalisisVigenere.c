#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_TEXT 1000000
#define ALPHABET 26
#define A 'A'

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

#define MAX_K_CAND 30   // probamos tamaños de clave 2..30 (ajústalo si quieres más)

// Trigrama codificado + posición para ordenar y agrupar
typedef struct {
    int key;  // (A*26 + B)*26 + C
    int pos;  // índice en el texto
} Trip;

// Comparador para qsort: primero por key, luego por pos
static int cmp_trip(const void *pa, const void *pb) {
    const Trip *a = (const Trip*)pa, *b = (const Trip*)pb;
    if (a->key != b->key) return (a->key < b->key) ? -1 : 1;
    return (a->pos - b->pos);
}

// Codifica 3 letras A..Z en un entero [0, 26^3)
static inline int enc3(char a, char b, char c) {
    return ((a - A) * 26 + (b - A)) * 26 + (c - A);
}

// Kasiski robusto con histograma de divisores
void kasiski(const char *text, int len) {
    printf("=== Test de Kasiski (robusto) ===\n");

    if (len < 6) {
        printf("Texto demasiado corto para Kasiski.\n");
        return;
    }

    // Construimos la lista de trigramas (clave + posición)
    int ntrip = len - 2;
    Trip *arr = (Trip*)malloc(sizeof(Trip) * ntrip);
    if (!arr) { fprintf(stderr, "Sin memoria.\n"); return; }

    for (int i = 0; i < ntrip; i++) {
        arr[i].key = enc3(text[i], text[i+1], text[i+2]);
        arr[i].pos = i;
    }

    // Ordenamos por clave y posición para agrupar repeticiones
    qsort(arr, ntrip, sizeof(Trip), cmp_trip);

    // Histograma de divisores: votos para cada posible tamaño de clave
    int votes[MAX_K_CAND + 1] = {0};

    // Recorremos grupos de mismo trigrama
    int i = 0;
    while (i < ntrip) {
        int j = i + 1;
        while (j < ntrip && arr[j].key == arr[i].key) j++;

        int group_sz = j - i;
        if (group_sz >= 2) {
            // Distancias entre apariciones consecutivas y MCD por trigrama
            int g = 0;
            for (int t = i + 1; t < j; t++) {
                int d = arr[t].pos - arr[t - 1].pos;   // siempre >0
                //Ignoramos distancias demasiado pequeñas para reducir ruido
                if (d <= 40 || d >= len/2) continue;
                // Acumulamos MCD del grupo
                g = (g == 0) ? d : mcd(g, d);

                // Votamos todos los divisores candidatos
                for (int k = 2; k <= MAX_K_CAND; k++) {
                    if (d % k == 0) votes[k]++;
                }
            }

            // Mensaje informativo por grupo (solo si aporta algo)
            if (g > 1) {
                int p = arr[i].pos;
                char trig[4] = { text[p], text[p+1], text[p+2], '\0' };
                printf("Trigrama %s (repite %d veces) -> MCD grupo: %d\n", trig, group_sz, g);
            }
        }

        i = j;
    }

    free(arr);

    // Elegimos el tamaño con más votos
    int best_k = 0, best_votes = 0;
    for (int k = 2; k <= MAX_K_CAND; k++) {
        if (votes[k] > best_votes) {
            best_votes = votes[k];
            best_k = k;
        }
    }

    if (best_k == 0) {
        printf("\nNo hay suficientes repeticiones significativas (o todas irrelevantes). "
               "Prueba con otro n-grama (>3) o usa el IC.\n");
        return;
    }

    // Mostramos el top de candidatos (opcional)
    printf("\nVotos por candidato (2..%d):\n", MAX_K_CAND);
    for (int k = 2; k <= MAX_K_CAND; k++) {
        if (votes[k] > 0) printf("  %2d -> %d\n", k, votes[k]);
    }

    printf("\nEstimación longitud de la clave (Kasiski): %d (votos = %d)\n", best_k, best_votes);
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
