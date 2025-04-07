#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"
#include "semantico.h"

void analizar_palabra_clave(Nodo* arbol, const char* linea, int *num_linea, bool fromF_Or_P, FILE* archivo);
void analizar_cabecera_funcion(Nodo* arbol, char* linea, int num_linea, char* nombre_funcion);
void analizar_funcion(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* nombre_funcion);
void analizar_expresion(Nodo* arbol, char* expr, int num_linea);
void analizar_asignacion(Nodo* arbol, const char* linea, int num_linea);
void imprimir_arbol(Nodo* nodo, int nivel);
void analizar_writeln(Nodo* arbol, const char* linea, int num_linea);
void analizar_if(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo);
void analizar_while(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo);
void analizar_for(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo);

#endif
