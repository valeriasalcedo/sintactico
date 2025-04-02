#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

typedef struct Nodo {
    char tipo[20];
    char valor[50];
    struct Nodo** hijos;
    int num_hijos;
} Nodo;

Nodo* crear_nodo(const char* tipo, const char* valor);
void agregar_hijo(Nodo* padre, Nodo* hijo);
void imprimir_arbol(Nodo* nodo, int nivel);
void liberar_nodo(Nodo* nodo);
void analizar_archivo(FILE* archivo, Nodo* arbol);

#endif