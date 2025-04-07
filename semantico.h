// semantico.h
#ifndef SEMANTICO_H
#define SEMANTICO_H

#include <stdbool.h>

typedef enum {
    TIPO_INTEGER,
    TIPO_STRING,
    TIPO_REAL,
    TIPO_BOOLEAN,
    TIPO_CHAR,
    TIPO_DESCONOCIDO
} TipoDato;

void mostrar_error(const char* mensaje, int linea, const char* detalle);
void mostrar_advertencia(const char* mensaje, int linea);
bool end_with_semicolon(const char* str);
void trim_semicolon(char *str);
void trim(char *str);
void toLowerCase(char *str);
bool starts_with(const char* str, const char* prefix);
TipoDato obtener_tipo_desde_string(const char* tipo_str);
bool es_palabra_clave_similar(const char* palabra, int num_linea);

#endif
