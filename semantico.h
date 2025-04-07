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

typedef struct Nodo {
    char tipo[20];
    char valor[50];
    struct Nodo** hijos;
    int num_hijos;
} Nodo;

Nodo* crear_nodo(const char* tipo, const char* valor);
void agregar_hijo(Nodo* padre, Nodo* hijo);

TipoDato obtener_tipo_desde_string(const char* tipo_str);
void inicializar_tabla_simbolos();
void agregar_variable(const char* nombre, TipoDato tipo, int linea);
void agregar_funcion(const char* nombre, TipoDato tipo_retorno, int linea);
bool variable_existe(const char* nombre);
bool funcion_existe(const char* nombre);
void marcar_variable_inicializada(const char* nombre);
void marcar_funcion_con_retorno(const char* nombre);
bool verificar_tipos_compatibles(TipoDato tipo1, TipoDato tipo2);
TipoDato inferir_tipo_expresion(const char* expr);
bool es_llamada_funcion(const char* expr);
void procesar_llamada_funcion(const char* expr, int num_linea);

#endif
