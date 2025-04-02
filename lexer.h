#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>

bool es_palabra_clave(const char* palabra);
bool es_tipo_valido(const char* tipo);
bool validar_asignacion(const char* linea, int num_linea);
bool validar_condicion(const char* condicion, int num_linea);
bool validar_parametros_funcion(const char* parametros, int num_linea);

#endif
