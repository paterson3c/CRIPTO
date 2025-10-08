#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEXT 1000000
#define ALPHABET 26

#define MAX_K_CAND 40 // Número máximo de candidatos a longitud de clave (2..40)
#define MIN_DIST 20   // Distancia mínima entre repeticiones a considerar
#define NGRAM 3       // Tamaño del n-grama
#define A 'A'         // Valor ASCII base para las letras mayúsculas

// Función para limpiar el texto (solo A-Z)
int load_text(const char *filename, char *buffer)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        perror("Error abriendo fichero");
        exit(EXIT_FAILURE);
    }
    int len = 0, c;
    while ((c = fgetc(f)) != EOF)
    {
        if (isalpha(c))
            buffer[len++] = toupper(c);
    }
    buffer[len] = '\0';
    fclose(f);
    return len;
}

// --------------------------------------------------------
// Función para calcular el Máximo Común Divisor (MCD)
int mcd(int a, int b)
{
    while (b != 0)
    {                // Repite hasta que el divisor sea cero
        int tmp = b; // Guarda el divisor actual
        b = a % b;   // Calcula el resto de la división
        a = tmp;     // Actualiza el dividendo
    }
    return a; // Devuelve el último divisor no nulo
}

// --------------------------------------------------------
// Convierte un n-grama (por ejemplo, "ABCD") en un número entero único
// útil para comparar y ordenar n-gramas fácilmente
static inline int encN(const char *s, int n)
{
    int val = 0;
    for (int i = 0; i < n; i++)
    {
        val = val * 26 + (s[i] - A); // Convierte cada letra en base 26
    }
    return val;
}

// --------------------------------------------------------
// Estructura para almacenar un n-grama codificado y su posición en el texto
typedef struct
{
    int key; // valor entero del n-grama
    int pos; // posición donde aparece en el texto
} Ngram;

// Función de comparación para qsort (ordena por clave y posición)
static int cmp_ngram(const void *a, const void *b)
{
    const Ngram *x = (const Ngram *)a, *y = (const Ngram *)b;
    if (x->key != y->key)
        return (x->key < y->key) ? -1 : 1; // Ordena por clave
    return x->pos - y->pos;                // Si clave igual, por posición
}

// --------------------------------------------------------
// Función principal del Test de Kasiski
void kasiski(const char *text, int len)
{
    printf("=== Test de Kasiski ===\n");

    // Verifica que el texto sea lo suficientemente largo
    if (len < NGRAM + 3)
    {
        printf("Texto demasiado corto para analizar.\n");
        return;
    }

    // Crea un array de n-gramas para todo el texto
    int total = len - (NGRAM - 1);              // Total de n-gramas posibles
    Ngram *arr = malloc(total * sizeof(Ngram)); // Reserva memoria dinámica
    if (!arr)
    {
        fprintf(stderr, "Error: sin memoria.\n");
        return;
    }

    // Llena el array con los n-gramas codificados y sus posiciones
    for (int i = 0; i < total; i++)
    {
        arr[i].key = encN(text + i, NGRAM);
        arr[i].pos = i;
    }

    // Ordena el array por clave (así las repeticiones quedan contiguas)
    qsort(arr, total, sizeof(Ngram), cmp_ngram);

    // Inicializa el histograma de votos (posibles longitudes de clave)
    int votes[MAX_K_CAND + 1] = {0};

    // Recorre los grupos de n-gramas iguales para calcular distancias
    int i = 0;
    while (i < total)
    {
        int j = i + 1;
        while (j < total && arr[j].key == arr[i].key)
            j++; // Agrupa repeticiones

        int group_sz = j - i; // Tamaño del grupo (veces que se repite el n-grama)
        if (group_sz >= 2)
        {              // Solo interesa si se repite más de una vez
            int g = 0; // MCD acumulado del grupo

            // Calcula distancias entre la primera aparición y todas las siguientes del mismo n-grama
            int base_pos = arr[i].pos; // posición de la primera aparición

            for (int t = i + 1; t < j; t++)
            {
                int d = arr[t].pos - base_pos; // distancia desde la primera aparición

                // Filtro para descartar distancias irrelevantes o demasiado grandes
                if (d < MIN_DIST || d >= len / 2)
                    continue;

                // Calcula el MCD acumulado del grupo
                g = (g == 0) ? d : mcd(g, d);

                // // Vota por todos los divisores razonables de esta distancia
                // for (int k = 2; k <= MAX_K_CAND; k++)
                // {
                //     if (d % k == 0)
                //         votes[k]++;
                // }
            }

            // Filtro adicional: solo sumar votos para MCDs razonables (entre 2 y 20)
            if (g >= 2 && g <= 20)
                votes[g]++;

            // Muestra información del grupo si hay un MCD válido
            if (g > 1)
            {
                char s[NGRAM + 1]; // Extrae el n-grama en texto legible
                for (int t = 0; t < NGRAM; t++)
                    s[t] = text[arr[i].pos + t];
                s[NGRAM] = '\0';
                printf("N-grama %s (repite %d veces) -> MCD grupo: %d\n", s, group_sz, g);
            }
        }
        i = j; // Avanza al siguiente grupo
    }

    // Determina la longitud de clave más votada
    int best_k = 0, best_votes = 0;
    printf("\nVotos por longitud candidata:\n");
    for (int k = 2; k <= MAX_K_CAND; k++)
    {
        if (votes[k] > 0)
            printf("  %2d -> %d\n", k, votes[k]); // Muestra el número de votos

        if (votes[k] > best_votes)
        { // Guarda el mejor candidato
            best_votes = votes[k];
            best_k = k;
        }
    }

    // Imprime la longitud de clave más probable
    if (best_k > 0)
        printf("\n>>> Estimación de longitud de la clave: %d (votos = %d)\n", best_k, best_votes);
    else
        printf("\nNo se encontraron repeticiones útiles para deducir la longitud.\n");

    // Libera memoria usada
    free(arr);
}

// Índice de coincidencia para longitud n
void ic(const char *text, int len, int n)
{
    printf("=== Índice de Coincidencia para n=%d ===\n", n);
    for (int k = 0; k < n; k++)
    {
        int freq[ALPHABET] = {0};
        int count = 0;
        for (int i = k; i < len; i += n)
        {
            freq[text[i] - A]++;
            count++;
        }
        double ic_val = 0.0;
        for (int j = 0; j < ALPHABET; j++)
        {
            ic_val += freq[j] * (freq[j] - 1);
        }
        ic_val /= (double)(count * (count - 1));
        printf("Subcadena %d: IC = %.3f\n", k + 1, ic_val);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Uso: %s {-kasiski | -ic N} -i filein\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filein = NULL;
    int n = 0;
    int mode = 0; // 1=kasiski, 2=ic

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-kasiski") == 0)
            mode = 1;
        else if (strcmp(argv[i], "-ic") == 0 && i + 1 < argc)
        {
            mode = 2;
            n = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
            filein = argv[++i];
    }

    if (!filein || mode == 0)
    {
        fprintf(stderr, "Parámetros incorrectos. Uso: %s {-kasiski | -ic N} -i filein\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *text = malloc(MAX_TEXT);
    int len = load_text(filein, text);

    if (mode == 1)
        kasiski(text, len);
    else if (mode == 2)
        ic(text, len, n);

    free(text);
    return 0;
}
