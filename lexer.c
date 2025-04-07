// lexer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer.h"

const char* palabras_reservadas[] = {
    "var", "begin", "end", "if", "then", "else", "while", "do", "for", "to", "downto", "repeat", "until", "writeln", "integer", "real"
};

bool es_reservada(const char* palabra) {
    for (int i = 0; i < sizeof(palabras_reservadas) / sizeof(palabras_reservadas[0]); i++) {
        if (strcmp(palabra, palabras_reservadas[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool es_entero(const char* palabra) {
    for (int i = 0; palabra[i]; i++) {
        if (!isdigit(palabra[i])) return false;
    }
    return true;
}

bool es_real(const char* palabra) {
    int punto = 0;
    for (int i = 0; palabra[i]; i++) {
        if (palabra[i] == '.') punto++;
        else if (!isdigit(palabra[i])) return false;
    }
    return punto == 1;
}

bool es_cadena(const char* palabra) {
    int len = strlen(palabra);
    return len >= 2 && palabra[0] == '\'' && palabra[len - 1] == '\'';
}

void analizar_linea_lexico(const char* linea, int numero_linea) {
    printf("[LINEA %d]: %s\n", numero_linea, linea);

    // Extraer cadenas primero para evitar tokenizar sus partes
    const char* p = linea;
    while ((p = strchr(p, '\'')) != NULL) {
        const char* fin = strchr(p + 1, '\'');
        if (fin) {
            int len = fin - p + 1;
            char* cadena = (char*)malloc(len + 1);
            strncpy(cadena, p, len);
            cadena[len] = '\0';
            printf("[CADENA] %s\n", cadena);
            free(cadena);
            p = fin + 1;
        } else {
            break;
        }
    }

    // Crear una copia sin cadenas para tokenizar
    char sin_cadenas[256];
    int i = 0, j = 0;
    bool dentro_cadena = false;
    while (linea[i] != '\0') {
        if (linea[i] == '\'') {
            dentro_cadena = !dentro_cadena;
            sin_cadenas[j++] = ' ';
        } else {
            sin_cadenas[j++] = dentro_cadena ? ' ' : linea[i];
        }
        i++;
    }
    sin_cadenas[j] = '\0';

    const char* delimitadores = " \t\n;():=+-*/<>", *delim = ";():=+-*/<>";
    char buffer[256];
    strcpy(buffer, sin_cadenas);

    char* token = strtok(buffer, delimitadores);
    while (token != NULL) {
        if (es_reservada(token)) {
            printf("[RESERVADA] %s\n", token);
        } else if (es_entero(token)) {
            printf("[ENTERO] %s\n", token);
        } else if (es_real(token)) {
            printf("[REAL] %s\n", token);
        } else {
            printf("[IDENTIFICADOR] %s\n", token);
        }
        token = strtok(NULL, delimitadores);
    }
}

void analizar_archivo_lexico(FILE* archivo) {
    printf("===== ANÁLISIS LÉXICO =====\n");
    char linea[256];
    int num_linea = 1;
    while (fgets(linea, sizeof(linea), archivo)) {
        analizar_linea_lexico(linea, num_linea++);
    }
    rewind(archivo);
}