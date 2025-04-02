#include "lexer.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

const char* palabras_clave[] = {"begin", "end", "then", "else", "while", "do", "for", "to", "downto", "repeat", "until", "case", "of", "const", "type", "record", "array", "var"};
const char* tipos_validos[] = {"integer", "string", "real", "boolean", "char"};

bool es_palabra_clave(const char* palabra) {
    for (int i = 0; i < sizeof(palabras_clave) / sizeof(palabras_clave[0]); i++) {
        if (strcmp(palabra, palabras_clave[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool es_tipo_valido(const char* tipo) {
    for (int i = 0; i < sizeof(tipos_validos) / sizeof(tipos_validos[0]); i++) {
        if (strcmp(tipo, tipos_validos[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool validar_asignacion(const char* linea, int num_linea) {
    return strstr(linea, ":=") != NULL;
}

bool validar_condicion(const char* condicion, int num_linea) {
    return strstr(condicion, "=") || strstr(condicion, "<") || strstr(condicion, ">");
}

bool validar_parametros_funcion(const char* parametros, int num_linea) {
    return strchr(parametros, ':') != NULL;
}