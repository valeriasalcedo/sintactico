#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"      
#include "lexer.h"
#include "semantico.h"


// Tabla de símbolos global
TablaSimbolos tabla;

// Declaración de arreglos y constantes válidas
const char* TIPOS_VALIDOS[] = {"integer", "string", "real", "boolean", "char"};
const char* OPERADORES_ARITMETICOS[] = {"*", "/", "mod", "+", "-"};
const int CANTIDAD_TIPOS_VALIDOS = 5;
const int CANTIDAD_OPERADORES_ARITMETICOS = 5;

// Inicialización de la tabla
void inicializar_tabla_simbolos() {
    tabla.num_variables = 0;
    tabla.num_funciones = 0;
    strcpy(tabla.ambito_actual, "global");
}

// Conversión de string a TipoDato
TipoDato obtener_tipo_desde_string(const char* tipo_str) {
    if (strcmp(tipo_str, "integer") == 0) return TIPO_INTEGER;
    if (strcmp(tipo_str, "string") == 0) return TIPO_STRING;
    if (strcmp(tipo_str, "real") == 0) return TIPO_REAL;
    if (strcmp(tipo_str, "boolean") == 0) return TIPO_BOOLEAN;
    if (strcmp(tipo_str, "char") == 0) return TIPO_CHAR;
    return TIPO_DESCONOCIDO;
}

// Operaciones sobre la tabla de símbolos
void agregar_variable(const char* nombre, TipoDato tipo, int linea) {
    if (tabla.num_variables < 100) {
        strcpy(tabla.variables[tabla.num_variables].nombre, nombre);
        tabla.variables[tabla.num_variables].tipo = tipo;
        tabla.variables[tabla.num_variables].inicializada = false;
        tabla.variables[tabla.num_variables].linea_declaracion = linea;
        tabla.num_variables++;
    }
}

void agregar_funcion(const char* nombre, TipoDato tipo_retorno, int linea) {
    if (tabla.num_funciones < 50) {
        strcpy(tabla.funciones[tabla.num_funciones].nombre, nombre);
        tabla.funciones[tabla.num_funciones].tipo_retorno = tipo_retorno;
        tabla.funciones[tabla.num_funciones].num_parametros = 0;
        tabla.funciones[tabla.num_funciones].retorno_asignado = false;
        tabla.funciones[tabla.num_funciones].linea_declaracion = linea;
        tabla.funciones[tabla.num_funciones].tiene_retorno = false;
        tabla.num_funciones++;
    }
}

void agregar_parametro_funcion(const char* nombre_funcion, const char* nombre_param, TipoDato tipo) {
    for (int i = 0; i < tabla.num_funciones; i++) {
        if (strcmp(tabla.funciones[i].nombre, nombre_funcion) == 0) {
            int n = tabla.funciones[i].num_parametros;
            strcpy(tabla.funciones[i].parametros[n].nombre, nombre_param);
            tabla.funciones[i].parametros[n].tipo = tipo;
            tabla.funciones[i].parametros[n].inicializada = true;
            tabla.funciones[i].num_parametros++;
            return;
        }
    }
}

bool variable_existe(const char* nombre) {
    for (int i = 0; i < tabla.num_variables; i++) {
        if (strcmp(tabla.variables[i].nombre, nombre) == 0) return true;
    }
    return false;
}

bool funcion_existe(const char* nombre) {
    for (int i = 0; i < tabla.num_funciones; i++) {
        if (strcmp(tabla.funciones[i].nombre, nombre) == 0) return true;
    }
    return false;
}

Variable* buscar_variable(const char* nombre) {
    for (int i = 0; i < tabla.num_variables; i++) {
        if (strcmp(tabla.variables[i].nombre, nombre) == 0) return &tabla.variables[i];
    }
    return NULL;
}

Funcion* buscar_funcion(const char* nombre) {
    for (int i = 0; i < tabla.num_funciones; i++) {
        if (strcmp(tabla.funciones[i].nombre, nombre) == 0) return &tabla.funciones[i];
    }
    return NULL;
}

void marcar_variable_inicializada(const char* nombre) {
    Variable* var = buscar_variable(nombre);
    if (var) var->inicializada = true;
}

void marcar_funcion_con_retorno(const char* nombre) {
    Funcion* func = buscar_funcion(nombre);
    if (func) func->retorno_asignado = true;
}

// Verificación de tipos
bool verificar_tipos_compatibles(TipoDato tipo1, TipoDato tipo2) {
    if (tipo1 == tipo2) return true;
    if ((tipo1 == TIPO_INTEGER && tipo2 == TIPO_REAL) ||
        (tipo1 == TIPO_REAL && tipo2 == TIPO_INTEGER)) return true;
    return false;
}

TipoDato inferir_tipo_expresion(const char* expr) {
    if (strstr(expr, "\"") || strstr(expr, "'")) return TIPO_STRING;
    if (strstr(expr, ".")) return TIPO_REAL;
    if (strcmp(expr, "true") == 0 || strcmp(expr, "false") == 0) return TIPO_BOOLEAN;

    Variable* var = buscar_variable(expr);
    if (var) return var->tipo;

    return TIPO_INTEGER;
}

// Manejo de errores
void mostrar_error(const char* mensaje, int linea, const char* detalle) {
    fprintf(stderr, "Error en la línea %d: %s -> %s\n", linea + 1, mensaje, detalle);
    exit(1);
}

void mostrar_advertencia(const char* mensaje, int linea) {
    fprintf(stderr, "Advertencia en la línea %d: %s\n", linea + 1, mensaje);
}

// Procesamiento de llamadas a funciones
void procesar_llamada_funcion(const char* expr, int num_linea) {
    char nombre_funcion[50] = {0};
    int i = 0;
    while (expr[i] && expr[i] != '(' && i < 49) {
        nombre_funcion[i] = expr[i];
        i++;
    }
    nombre_funcion[i] = '\0';
    convertir_minusculas(nombre_funcion);

    Funcion* func = buscar_funcion(nombre_funcion);
    if (!func) {
        mostrar_error("Función no declarada", num_linea, nombre_funcion);
        return;
    }

    char* argumentos = extraer_argumentos_funcion(expr);
    int count = 0;
    char** args = argumentos ? split(argumentos, ",", &count) : NULL;

    if (count != func->num_parametros) {
        char msg[100];
        sprintf(msg, "Cantidad de argumentos incorrecta. Esperados: %d, Recibidos: %d", func->num_parametros, count);
        mostrar_error(msg, num_linea, expr);
    }

    for (int i = 0; i < count; i++) {
        limpiar_espacios(args[i]);
        TipoDato tipo_arg = inferir_tipo_expresion(args[i]);
        TipoDato tipo_param = func->parametros[i].tipo;
        if (!verificar_tipos_compatibles(tipo_param, tipo_arg)) {
            char msg[100];
            sprintf(msg, "Tipo incompatible en argumento %d. Esperado: %d, Recibido: %d", i+1, tipo_param, tipo_arg);
            mostrar_error(msg, num_linea, args[i]);
        }
    }

    for (int i = 0; i < count; i++) free(args[i]);
    free(args);
    free(argumentos);
}
