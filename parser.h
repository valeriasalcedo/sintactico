#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

// Definición del nodo del AST
typedef struct Nodo {
    char tipo[32];
    char valor[128];
    struct Nodo* hijos[10];
    int num_hijos;
} Nodo;

// Funciones para crear, agregar hijos e imprimir el AST
Nodo* crear_nodo(const char* tipo, const char* valor);
void agregar_hijo(Nodo* padre, Nodo* hijo);
void imprimir_ast(Nodo* nodo, int nivel);

// Función para analizar el archivo y generar el AST
void analizar_archivo(FILE* archivo, Nodo* raiz);

#endif
