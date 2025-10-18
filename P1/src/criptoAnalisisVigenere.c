#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEXT 1000000
#define ALPHABET 26

#define MAX_K_CAND 30 // Número máximo de candidatos a longitud de clave (2..40)
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

// ===== Ajustes robustos para ataque por IC + M(k) =====
// - Alfabeto de 26 letras (A-Z). Ñ y no-letras NO se cifran.
// - Para formar las subcolumnas usamos un contador que avanza SOLO en A-Z,
//   así las columnas quedan alineadas exactamente como en tu vigenere.c.

// --- Frecuencias ES/EN (porcentajes de la práctica) ---
static const double FREQ_ES_PCT[26] = {
    11.96,0.92,2.92,6.87,16.78,0.52,0.73,0.89,4.15,0.30,0.00,8.37,2.12,7.01,
    8.69,2.77,1.53,4.94,7.88,3.31,4.80,0.39,0.00,0.06,1.54,0.15
};
static const double FREQ_EN_PCT[26] = {
    8.04,1.54,3.06,3.99,12.51,2.30,1.96,5.49,7.26,0.16,0.67,4.14,2.53,7.09,
    7.60,2.00,0.11,6.12,6.54,9.25,2.71,0.99,1.92,0.19,1.73,0.19
};

static inline int is_letter26(char c) {
    // A-Z sin Ñ (tu cifrado solo avanza en estas)
    return (c >= 'A' && c <= 'Z' && c != 'Ñ');
}

static double load_language_probs(const char *lang, double P[26]) {
    const double *src = (lang && (strcmp(lang,"en")==0 || strcmp(lang,"EN")==0))
                        ? FREQ_EN_PCT : FREQ_ES_PCT; // por defecto ES
    double sum = 0.0, ic = 0.0;
    for (int i = 0; i < 26; ++i) sum += src[i];
    if (sum <= 0.0) sum = 1.0;
    for (int i = 0; i < 26; ++i) {
        P[i] = src[i] / sum;      // prob idioma
        ic  += P[i] * P[i];       // IC teórico ΣP_i^2
    }
    return ic;
}

// Recolecta frecuencias de la subcolumna k (0..n-1) para una clave de longitud n,
// recorriendo TODO el texto pero incrementando el índice de columna SOLO en A-Z (sin Ñ).
// Devuelve N (longitud de la subcolumna).
static int column_freq(const char *text, int len, int n, int k, int freq[26]) {
    memset(freq, 0, 26 * sizeof(int));
    int col_idx = 0; // avanza solo cuando vemos A-Z (sin Ñ)
    int N = 0;
    for (int i = 0; i < len; ++i) {
        char c = text[i];
        if (!is_letter26(c)) continue;      // ignoramos Ñ y no-letras (no avanzan clave)
        if ((col_idx % n) == k) {
            freq[c - 'A']++;
            N++;
        }
        col_idx++;
    }
    return N;
}

// IC medio para un n dado usando las subcolumnas "reales" (con la lógica anterior)
static double ic_for_n(const char *text, int len, int n) {
    double sum_ic = 0.0;
    int cols = 0;
    for (int k = 0; k < n; ++k) {
        int f[26]; int N = column_freq(text, len, n, k, f);
        if (N < 2) { cols++; continue; }
        long long num = 0;
        for (int j = 0; j < 26; ++j) num += 1LL * f[j] * (f[j] - 1);
        long long den = 1LL * N * (N - 1);
        sum_ic += (den ? (double)num / (double)den : 0.0);
        cols++;
    }
    return (cols ? sum_ic / cols : 0.0);
}

// M(k) = Σ_j P_j * ( f_{j+k} / ℓ )
// **ℓ es la longitud de ESA subcolumna** (errata corregida: no es ℓ/n).
// Recordatorio: como C = P + K, la subclave de CIFRADO coincide con el k que MAXIMIZA M(k)
// cuando comparamos P_j con la distribución del cifrado desplazada +k.
static int best_shift_M_for_column(const char *text, int len, int n, int kcol, const double P[26]) {
    int f[26]; int N = column_freq(text, len, n, kcol, f);
    if (N == 0) return 0;
    int best_k = 0;
    double best_s = -1e300;
    for (int k = 0; k < 26; ++k) {
        double s = 0.0;
        for (int j = 0; j < 26; ++j) {
            int idx = (j + k) % 26;         // f_{j+k}
            s += P[j] * ((double)f[idx] / (double)N);  // dividir por ℓ = N  ← errata arreglada
        }
        if (s > best_s + 1e-12 || (fabs(s - best_s) <= 1e-12 && k < best_k)) {
            best_s = s; best_k = k;
        }
    }
    return best_k; // letra de CIFRADO = 'A' + best_k
}

void vigenere_ic_attack(const char *text, int len, int max_k, const char *lang, char *out_key) {
    double P[26];
    double ic_lang = load_language_probs(lang, P);
    const double ic_uniform = 1.0 / 26.0;

    printf("=== Ataque Vigenere por IC (%s) ===\n", (lang && strcmp(lang,"en")==0) ? "EN" : "ES");
    printf("IC(teorico idioma)=%.5f, IC(aleatorio)=%.5f\n\n", ic_lang, ic_uniform);

    if (max_k < 1) max_k = 1;
    if (max_k > 60) max_k = 60;

    // 1) Estimar n por IC medio (con columnas reales que saltan Ñ y no-letras)
    int best_n = 1; double best_dist = 1e300; const double EPS = 5e-5;
    printf("IC medio por n:\n");
    for (int n = 1; n <= max_k; ++n) {
        double avg_ic = ic_for_n(text, len, n);
        double dist   = fabs(avg_ic - ic_lang);
        printf("  n=%2d -> ICmedio=%.5f (dist=%.5f)\n", n, avg_ic, dist);
        if (dist + EPS < best_dist || (fabs(dist - best_dist) <= EPS && n < best_n)) {
            best_dist = dist; best_n = n;
        }
    }

    printf("\n>>> Estimación de longitud de clave: n = %d\n", best_n);

    // 2) Subclaves con M(k) correcto (divide por ℓ y usa f_{j+k})
    for (int i = 0; i < best_n; ++i) {
        int k = best_shift_M_for_column(text, len, best_n, i, P);
        out_key[i] = (char)('A' + k);  // clave de CIFRADO (tu vigenere.c usa C = P + K)
        printf("  Subclave[%d] = %c (k=%d)\n", i+1, out_key[i], k);
    }
    out_key[best_n] = '\0';

    // 3) Reducir al periodo mínimo si se repite patrón
    int period = best_n;
    for (int d = 1; d <= best_n/2; ++d) {
        if (best_n % d) continue;
        int ok = 1;
        for (int i = d; i < best_n; ++i) if (out_key[i] != out_key[i % d]) { ok = 0; break; }
        if (ok) { period = d; break; }
    }
    if (period < best_n) {
        out_key[period] = '\0';
        printf(">>> Clave reducida al periodo detectado: %s (periodo %d)\n", out_key, period);
    } else {
        printf(">>> Clave estimada: %s\n", out_key);
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
    char clave[40];
    if (mode == 1)
        kasiski(text, len);
    else if (mode == 2)

        vigenere_ic_attack(text, len, MAX_K_CAND, "es", clave);

    free(text);
    return 0;
}
