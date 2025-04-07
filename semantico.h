// semantico.h
#ifndef SEMANTICO_H
#define SEMANTICO_H

#include <stdbool.h>

#define MAX_FUNCIONES 50

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
    Funcion funciones[MAX_FUNCIONES];
    int num_funciones;
    char ambito_actual[50]; 
} TablaSimbolos;

extern TablaSimbolos tabla;

// Declaraciones de funciones
TipoDato obtener_tipo_desde_string(const char* tipo_str);
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

#endif
