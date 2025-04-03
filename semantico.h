#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "parser.h"
#include <stdbool.h>

// Tipos de datos aceptados
typedef enum {
    TIPO_INTEGER,
    TIPO_STRING,
    TIPO_REAL,
    TIPO_BOOLEAN,
    TIPO_CHAR,
    TIPO_DESCONOCIDO
} TipoDato;

typedef struct {
    char nombre[50];
    TipoDato tipo;
    bool inicializada;
    int linea_declaracion;
} Variable;

typedef struct {
    char nombre[50];
    TipoDato tipo_retorno;
    Variable parametros[20];
    int num_parametros;
    bool retorno_asignado;
    int linea_declaracion;
    bool tiene_retorno;
} Funcion;

typedef struct {
    Variable variables[100];
    int num_variables;
    Funcion funciones[50];
    int num_funciones;
    char ambito_actual[50];
} TablaSimbolos;

extern TablaSimbolos tabla;

// Funciones de manejo de la tabla de símbolos
void inicializar_tabla_simbolos();
void agregar_variable(const char* nombre, TipoDato tipo, int linea);
void agregar_funcion(const char* nombre, TipoDato tipo_retorno, int linea);
void agregar_parametro_funcion(const char* nombre_funcion, const char* nombre_param, TipoDato tipo);
bool variable_existe(const char* nombre);
bool funcion_existe(const char* nombre);
Variable* buscar_variable(const char* nombre);
Funcion* buscar_funcion(const char* nombre);
void marcar_variable_inicializada(const char* nombre);
void marcar_funcion_con_retorno(const char* nombre);
bool verificar_tipos_compatibles(TipoDato tipo1, TipoDato tipo2);
TipoDato inferir_tipo_expresion(const char* expr);
TipoDato obtener_tipo_desde_string(const char* tipo_str);

// Errores y advertencias
void mostrar_error(const char* mensaje, int linea, const char* detalle);
void mostrar_advertencia(const char* mensaje, int linea);

// Funciones relacionadas a funciones y llamadas
char* extraer_argumentos_funcion(const char* str);
bool es_llamada_funcion(const char* expr);
void procesar_llamada_funcion(const char* expr, int num_linea);

void analizar_expresion(Nodo* nodo, const char* expresion, int* num_linea);


#endif
