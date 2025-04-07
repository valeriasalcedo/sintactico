#include "semantico.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

TablaSimbolos tabla;

TipoDato obtener_tipo_desde_string(const char* tipo_str) {
    if (strcmp(tipo_str, "integer") == 0) return TIPO_INTEGER;
    if (strcmp(tipo_str, "string") == 0) return TIPO_STRING;
    if (strcmp(tipo_str, "real") == 0) return TIPO_REAL;
    if (strcmp(tipo_str, "boolean") == 0) return TIPO_BOOLEAN;
    if (strcmp(tipo_str, "char") == 0) return TIPO_CHAR;
    return TIPO_DESCONOCIDO;
}

void inicializar_tabla_simbolos() {
    tabla.num_variables = 0;
    tabla.num_funciones = 0;
    strcpy(tabla.ambito_actual, "global");
}

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
    if (tabla.num_funciones >= MAX_FUNCIONES) {
        fprintf(stderr, "Error: Número máximo de funciones alcanzado\n");
        exit(1);
    }
    
    char nombre_lower[50];
    strcpy(nombre_lower, nombre);
    toLowerCase(nombre_lower);
    
    strcpy(tabla.funciones[tabla.num_funciones].nombre, nombre_lower);
    tabla.funciones[tabla.num_funciones].tipo_retorno = tipo_retorno;
    tabla.funciones[tabla.num_funciones].num_parametros = 0;
    tabla.funciones[tabla.num_funciones].linea_declaracion = linea;
    tabla.funciones[tabla.num_funciones].tiene_retorno = false;
    tabla.num_funciones++;
}

void agregar_parametro_funcion(const char* nombre_funcion, const char* nombre_param, TipoDato tipo) {
    for (int i = 0; i < tabla.num_funciones; i++) {
        if (strcmp(tabla.funciones[i].nombre, nombre_funcion) == 0) {
            int num = tabla.funciones[i].num_parametros;
            strcpy(tabla.funciones[i].parametros[num].nombre, nombre_param);
            tabla.funciones[i].parametros[num].tipo = tipo;
            tabla.funciones[i].parametros[num].inicializada = true; 
            tabla.funciones[i].num_parametros++;
            return;
        }
    }
}

bool variable_existe(const char* nombre) {
    char lowerNombre[50];
    strcpy(lowerNombre, nombre);
    toLowerCase(lowerNombre);

    for (int i = 0; i < tabla.num_variables; i++) {
        char lowerVarName[50];
        strcpy(lowerVarName, tabla.variables[i].nombre);
        toLowerCase(lowerVarName);
        if (strcmp(lowerVarName, lowerNombre) == 0) {
            return true;
        }
    }
    return false;
}

bool funcion_existe(const char* nombre) {
    char lowerNombre[50];
    strcpy(lowerNombre, nombre);
    toLowerCase(lowerNombre);

    for (int i = 0; i < tabla.num_funciones; i++) {
        char lowerFuncName[50];
        strcpy(lowerFuncName, tabla.funciones[i].nombre);
        toLowerCase(lowerFuncName);
        if (strcmp(lowerFuncName, lowerNombre) == 0) {
            return true;
        }
    }
    return false;
}

Variable* buscar_variable(const char* nombre) {
    char lowerNombre[50];
    strcpy(lowerNombre, nombre);
    toLowerCase(lowerNombre);

    for (int i = 0; i < tabla.num_variables; i++) {
        char lowerVarName[50];
        strcpy(lowerVarName, tabla.variables[i].nombre);
        toLowerCase(lowerVarName);
        if (strcmp(lowerVarName, lowerNombre) == 0) {
            return &tabla.variables[i];
        }
    }
    return NULL;
}

Funcion* buscar_funcion(const char* nombre) {
    char nombre_lower[50];
    strcpy(nombre_lower, nombre);
    toLowerCase(nombre_lower);

    for (int i = 0; i < tabla.num_funciones; i++) {
        char func_nombre_lower[50];
        strcpy(func_nombre_lower, tabla.funciones[i].nombre);
        toLowerCase(func_nombre_lower);

        if (strcmp(func_nombre_lower, nombre_lower) == 0) {
            return &tabla.funciones[i];
        }
    }
    return NULL;
}

void marcar_variable_inicializada(const char* nombre) {
    Variable* var = buscar_variable(nombre);
    if (var) {
        var->inicializada = true;
    }
}

void marcar_funcion_con_retorno(const char* nombre) {
    Funcion* func = buscar_funcion(nombre);
    if (func) {
        func->retorno_asignado = true;
    }
}

bool verificar_tipos_compatibles(TipoDato tipo1, TipoDato tipo2) {
    if (tipo1 == tipo2) return true;

    if ((tipo1 == TIPO_INTEGER && tipo2 == TIPO_REAL) ||
        (tipo1 == TIPO_REAL && tipo2 == TIPO_INTEGER)) {
        return true;
    }

    if ((tipo1 == TIPO_CHAR && (tipo2 == TIPO_INTEGER || tipo2 == TIPO_REAL)) ||
        (tipo2 == TIPO_CHAR && (tipo1 == TIPO_INTEGER || tipo1 == TIPO_REAL))) {
        return false;
    }
    return false;
}

TipoDato inferir_tipo_expresion(const char* expr) {
    if (strstr(expr, "\"") != NULL || strstr(expr, "'") != NULL) {
        return TIPO_STRING;
    }

    if (strstr(expr, ".") != NULL) {
        return TIPO_REAL;
    }

    if (strcmp(expr, "true") == 0 || strcmp(expr, "false") == 0) {
        return TIPO_BOOLEAN;
    }

    Variable* var = buscar_variable(expr);
    if (var) {
        return var->tipo;
    }

    return TIPO_INTEGER;
}