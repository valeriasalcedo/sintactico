// semantico.c
#include "semantico.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void mostrar_error(const char* mensaje, int linea, const char* detalle) {
    fprintf(stderr, "Error en la linea %d: %s -> %s\n", linea + 1, mensaje, detalle);
    exit(1);
}

void mostrar_advertencia(const char* mensaje, int linea) {
    fprintf(stderr, "Advertencia en la linea %d: %s\n", linea + 1, mensaje);
}

bool end_with_semicolon(const char* str) {
    char copia[256];
    strcpy(copia, str);
    trim(copia);
    size_t len = strlen(copia);
    if (len == 0) return false;
    return copia[len - 1] == ';';
}

void trim_semicolon(char *str) {
    int len = strlen(str);
    while (len > 0 && str[len - 1] == ';') {
        str[len - 1] = '\0';
        len--;
    }
}

void trim(char *str) {
    char *start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    if (*start == '\0') {
        *str = '\0';
        return;
    }
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

void toLowerCase(char *str) {
    while (*str) {
        *str = tolower((unsigned char) *str);
        str++;
    }
}

bool starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

TipoDato obtener_tipo_desde_string(const char* tipo_str) {
    if (strcmp(tipo_str, "integer") == 0) return TIPO_INTEGER;
    if (strcmp(tipo_str, "string") == 0) return TIPO_STRING;
    if (strcmp(tipo_str, "real") == 0) return TIPO_REAL;
    if (strcmp(tipo_str, "boolean") == 0) return TIPO_BOOLEAN;
    if (strcmp(tipo_str, "char") == 0) return TIPO_CHAR;
    return TIPO_DESCONOCIDO;
}

bool es_palabra_clave_similar(const char* palabra, int num_linea) {
    const char* palabras_clave[] = {
        "begin", "end", "then", "else", "while", "do", "for", "to", "downto",
        "repeat", "until", "case", "of", "const", "type", "record", "array", "var",
        "function", "procedure"
    };
    int cantidad = sizeof(palabras_clave) / sizeof(palabras_clave[0]);

    char copia[256];
    strcpy(copia, palabra);
    int len = strlen(copia);
    if (len > 0 && (copia[len - 1] == '.' || copia[len - 1] == ';')) {
        copia[len - 1] = '\0';
    }

    for (int i = 0; i < cantidad; i++) {
        if (strcmp(copia, palabras_clave[i]) == 0) return false;
    }

    for (int i = 0; i < cantidad; i++) {
        int len1 = strlen(copia);
        int len2 = strlen(palabras_clave[i]);
        if (abs(len1 - len2) <= 1) {
            int differences = 0;
            int minLen = len1 < len2 ? len1 : len2;
            for (int j = 0; j < minLen && differences <= 2; j++) {
                if (copia[j] != palabras_clave[i][j]) differences++;
            }
            differences += abs(len1 - len2);
            if (differences <= 2) {
                char mensaje[128];
                sprintf(mensaje, "Posible error tipografico: '%s' (quiza quiso escribir '%s')", copia, palabras_clave[i]);
                mostrar_error(mensaje, num_linea, palabra);
                return true;
            }
        }
    }
    return false;
}