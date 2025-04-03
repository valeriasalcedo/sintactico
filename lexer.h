#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

// Palabras reservadas y operadores
extern const char* PALABRAS_RESERVADAS[];
extern const int CANTIDAD_PALABRAS_RESERVADAS;
extern const char* OPERADORES_COMPARACION[];
extern const int CANTIDAD_OPERADORES_COMPARACION;

// Funciones de utilidades de cadena y validaciones
void convertir_minusculas(char* str);
void limpiar_espacios(char* str);
void quitar_punto_y_coma(char* str);
bool comienza_con(const char* str, const char* prefijo);
bool termina_con(const char* str, const char* sufijo);
bool comienza_con_insensible(const char* str, const char* prefijo);
bool sugerencia_palabra_reservada(const char* palabra, int linea);
bool contiene_palabra_reservada(const char* cadena, const char* lista[], int cantidad);

// Validaciones
bool validar_asignacion(const char* linea, int num_linea);
bool validar_condicion(const char* condicion, int num_linea);
bool validar_parametros_funcion(const char* parametros, int num_linea);
bool validar_condicion_for(const char* linea, int num_linea);

// Split y manejo de argumentos
char** split(const char* str, const char* delim, int* cantidad);
void liberar_split(char** arreglo, int cantidad);
int contar_elementos(char** arreglo);

// Extracciones
char* extraer_parentesis(const char* str);
void contenido_writeln(char* linea, char* contenido);
void extraer_condicion_if(const char* linea, char* condicion);
void extraer_condicion_while(const char* linea, char* condicion);
void extraer_condicion_for(const char* linea, char* inicializacion, char* operador_control, char* final);
void obtener_nombre_funcion(const char* linea, char* nombre, size_t tam);
void obtener_nombre_procedure(const char* linea, char* nombre, size_t tam);

// Declaraciones necesarias del módulo semántico
bool es_llamada_funcion(const char* expr);
char* extraer_argumentos_funcion(const char* str);

#endif
