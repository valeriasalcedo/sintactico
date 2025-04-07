// lexer.h
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    TOKEN_IDENTIFICADOR,
    TOKEN_ENTERO,
    TOKEN_REAL,
    TOKEN_CADENA,
    TOKEN_CARACTER,
    TOKEN_RESERVADA,
    TOKEN_OPERADOR,
    TOKEN_DESCONOCIDO
} TipoToken;

typedef struct {
    TipoToken tipo;
    char valor[100];
} Token;

void analizar_linea_lexico(const char* linea, int numero_linea);
void analizar_archivo_lexico(FILE* archivo);
const char* tipo_token_str(TipoToken tipo);
bool es_palabra_reservada(const char* palabra);
bool es_operador(const char* palabra);
bool es_numero_entero(const char* str);
bool es_numero_real(const char* str);
bool es_cadena_valida(const char* str);
bool es_caracter_valido(const char* str);
void trim(char* str);
void toLowerCase(char* str);

#endif
