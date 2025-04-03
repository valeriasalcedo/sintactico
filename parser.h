#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>

// ==========================
// Estructura del Nodo del árbol
// ==========================
typedef struct Nodo {
    char tipo[50];
    char valor[256];
    struct Nodo** hijos;
    int num_hijos;
} Nodo;

// ==========================
// Funciones del Parser
// ==========================
Nodo* crear_nodo(const char* tipo, const char* valor);
void agregar_hijo(Nodo* padre, Nodo* hijo);
void imprimir_arbol(Nodo* nodo, int nivel);

void analizar_funcion(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* nombre_funcion);
void analizar_procedimiento(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo, char* nombre_procedimiento);
void analizar_variables(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* ultima_linea);
void analizar_condicional_si(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo);
void analizar_mientras(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo);
void analizar_para(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo);
void analizar_escribir(Nodo* arbol, const char* linea, int num_linea);
void analizar_asignacion_variable(Nodo* arbol, const char* linea, int num_linea);
void analizar_palabra_reservada(Nodo* arbol, const char* linea, int* num_linea, bool desde_funcion_o_procedimiento, FILE* archivo);

// Declaración agregada explícitamente
void analizar_expresion(Nodo* nodo, const char* expresion, int* num_linea);

#endif
