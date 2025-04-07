#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

bool end_with_semicolon(const char* str);
void removeSpaces(char *str);
void trim(char *str);
void trim_semicolon(char *str);
bool starts_with(const char* str, const char* prefix);
bool ends_with(const char* str, const char* suffix);
char **split(const char *str, const char *delim, int *count);
char **split_function(const char *str, int *count);
char *extraer_parentesis(const char *str);
char *extraer_argumentos_funcion(const char *str);
void toLowerCase(char *str);

#endif
