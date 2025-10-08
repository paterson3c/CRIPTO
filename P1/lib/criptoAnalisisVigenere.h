#ifndef CRIPTOANALISISVIGNERE_H
#define CRIPTOANALISISVIGNERE_H

// Función para limpiar el texto (solo A-Z)
int load_text(const char *filename, char *buffer);

/*Calcula el maximo común divisor de 2 números*/
int mcd(int a, int b);

// Test de Kasiski: busca repeticiones de trigramas y distancias
void kasiski(const char *text, int len);

// Índice de coincidencia para longitud n
void ic(const char *text, int len, int n);

#endif