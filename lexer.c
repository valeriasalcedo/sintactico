#include "lexer.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

bool end_with_semicolon(const char* str) {
    char temp[256];
    strcpy(temp, str);
    trim(temp);
    size_t len = strlen(temp);
    return temp[len - 1] == ';';
}

void removeSpaces(char *str) {
    char *dst = str; 
    while (*str) {
        if (!isspace((unsigned char)*str)) { 
            *dst++ = *str; 
        }
        str++;
    }
    *dst = '\0'; 
}

void trim(char *str) {
    char *start = str;
    char *end;

    while (*start && isspace((unsigned char)*start)) start++;
    if (*start == '\0') {
        *str = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;

    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

void trim_semicolon(char *str) {
    int len = strlen(str);
    while (len > 0 && str[len - 1] == ';') {
        str[len - 1] = '\0';
        len--;
    }
}

bool starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) return false;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return false;
    return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

char **split(const char *str, const char *delim, int *count) {
    char *copia = strdup(str); 
    int capacidad = 10; 
    char **resultado = (char **)malloc(capacidad * sizeof(char *)); 
    int index = 0;
    char *token = strtok(copia, delim);
    while (token != NULL) { 
        if (index >= capacidad) { 
            capacidad *= 2;
            resultado = (char **)realloc(resultado, capacidad * sizeof(char *));
        }
        resultado[index++] = strdup(token);  
        token = strtok(NULL, delim); 
    }
    resultado[index] = NULL;  
    *count = index; 
    free(copia); 
    return resultado; 
}
char **split_function(const char *str, int *count) {
    char *copia = strdup(str);
    char **resultado = (char **)malloc(2 * sizeof(char *));
    *count = 1;
    char *pos = strrchr(copia, ':');
    if (pos == NULL) {
        resultado[0] = strdup(copia);
        resultado[1] = NULL;
    } else {
        *pos = '\0';
        resultado[0] = strdup(copia);
        resultado[1] = strdup(pos + 1);
        *count = 2;
    }
    free(copia);
    return resultado;
}

char *extraer_parentesis(const char *str) {
    const char *inicio = strchr(str, '('); 
    const char *fin = strrchr(str, ')');   
    if (!inicio || !fin || inicio > fin) return NULL;
    size_t len = fin - inicio - 1; 
    char *resultado = (char *)malloc(len + 1);
    strncpy(resultado, inicio + 1, len); 
    resultado[len] = '\0';
    return resultado;
}

char* extraer_argumentos_funcion(const char* str) {
    const char* inicio = strchr(str, '(');
    if (!inicio) return NULL;
    const char* fin = strrchr(str, ')');
    if (!fin || fin < inicio) return NULL;
    size_t len = fin - inicio - 1;
    char* resultado = (char*)malloc(len + 1);
    strncpy(resultado, inicio + 1, len);
    resultado[len] = '\0';
    return resultado;
}

void toLowerCase(char *str) {
    while (*str) {
        *str = tolower((unsigned char) *str);
        str++;
    }
}
