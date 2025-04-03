#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "semantico.h"

const char* PALABRAS_RESERVADAS[] = {
    "begin", "end", "then", "else", "while", "do", "for", "to", "downto",
    "repeat", "until", "case", "of", "const", "type", "record", "array",
    "var", "function", "procedure"
};
const int CANTIDAD_PALABRAS_RESERVADAS = sizeof(PALABRAS_RESERVADAS) / sizeof(PALABRAS_RESERVADAS[0]);

const char* OPERADORES_COMPARACION[] = {
    "<>", "<=", ">=", "<", ">", "="
};
const int CANTIDAD_OPERADORES_COMPARACION = sizeof(OPERADORES_COMPARACION) / sizeof(OPERADORES_COMPARACION[0]);

void convertir_minusculas(char* str) {
    for (int i = 0; str[i]; i++) str[i] = tolower((unsigned char)str[i]);
}

void limpiar_espacios(char* str) {
    char *start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    if (*start == '\0') {
        *str = '\0';
        return;
    }
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

void quitar_punto_y_coma(char* str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ';' || str[len - 1] == '.')) {
        str[len - 1] = '\0';
        len--;
    }
}

bool comienza_con(const char* str, const char* prefijo) {
    return strncmp(str, prefijo, strlen(prefijo)) == 0;
}

bool termina_con(const char* str, const char* sufijo) {
    int len_str = strlen(str);
    int len_suf = strlen(sufijo);
    if (len_suf > len_str) return false;
    return strcmp(str + len_str - len_suf, sufijo) == 0;
}

bool comienza_con_insensible(const char* str, const char* prefijo) {
    size_t len = strlen(prefijo);
    for (size_t i = 0; i < len; i++) {
        if (tolower(str[i]) != tolower(prefijo[i])) return false;
    }
    return true;
}

char** split(const char* str, const char* delim, int* cantidad) {
    char* copia = strdup(str);
    int capacidad = 10;
    char** resultado = (char**)malloc(capacidad * sizeof(char*));
    int count = 0;

    char* token = strtok(copia, delim);
    while (token) {
        if (count >= capacidad) {
            capacidad *= 2;
            resultado = (char**)realloc(resultado, capacidad * sizeof(char*));
        }
        resultado[count++] = strdup(token);
        token = strtok(NULL, delim);
    }

    if (cantidad) *cantidad = count;
    resultado[count] = NULL;
    free(copia);
    return resultado;
}

void liberar_split(char** arreglo, int cantidad) {
    for (int i = 0; i < cantidad; i++) {
        free(arreglo[i]);
    }
    free(arreglo);
}

int contar_elementos(char** arreglo) {
    int count = 0;
    while (arreglo[count] != NULL) count++;
    return count;
}

bool sugerencia_palabra_reservada(const char* palabra, int linea) {
    if (strcmp(palabra, "en") == 0) {
        mostrar_error("¿Quiso decir 'end'?", linea, palabra);
        return true;
    }

    for (int i = 0; i < CANTIDAD_PALABRAS_RESERVADAS; i++) {
        int len1 = strlen(palabra);
        int len2 = strlen(PALABRAS_RESERVADAS[i]);
        if (abs(len1 - len2) > 2) continue;

        int diferencias = 0;
        for (int j = 0; j < len1 && j < len2; j++) {
            if (palabra[j] != PALABRAS_RESERVADAS[i][j]) diferencias++;
        }
        diferencias += abs(len1 - len2);

        if (diferencias <= 2) {
            char msg[100];
            sprintf(msg, "¿Quiso decir '%s'?", PALABRAS_RESERVADAS[i]);
            mostrar_error(msg, linea, palabra);
            return true;
        }
    }
    return false;
}

bool contiene_palabra_reservada(const char* cadena, const char* lista[], int cantidad) {
    char copia[256];
    strcpy(copia, cadena);
    char* token = strtok(copia, " ,.;()[]{}<>+-*/=!:\"\'\t\n\r");
    while (token) {
        for (int i = 0; i < cantidad; i++) {
            if (strcmp(token, lista[i]) == 0) return true;
        }
        token = strtok(NULL, " ,.;()[]{}<>+-*/=!:\"\'\t\n\r");
    }
    return false;
}

bool validar_asignacion(const char* linea, int num_linea) {
    if (strstr(linea, "=") && !strstr(linea, ":=")) {
        mostrar_error("Debe usar ':=' para asignar", num_linea, linea);
        return false;
    }

    if (strstr(linea, ":=")) {
        int count = 0;
        char** partes = split(linea, ":=", &count);
        if (!partes || count != 2) {
            liberar_split(partes, count);
            mostrar_error("Asignación mal formada", num_linea, linea);
            return false;
        }
        liberar_split(partes, count);
    }
    return true;
}

bool validar_condicion(const char* condicion, int num_linea) {
    for (int i = 0; i < CANTIDAD_OPERADORES_COMPARACION; i++) {
        if (strstr(condicion, OPERADORES_COMPARACION[i])) return true;
    }
    mostrar_error("Condición inválida, falta operador de comparación", num_linea, condicion);
    return false;
}

bool validar_parametros_funcion(const char* parametros, int num_linea) {
    char* copia = strdup(parametros);
    char* token = strtok(copia, ":");
    while (token != NULL) {
        if (strchr(token, ',')) {
            char* antes = token;
            while (isspace(*antes)) antes++;
            if (*antes == ',') {
                free(copia);
                mostrar_error("Falta variable antes de la coma", num_linea, parametros);
                return false;
            }
        }
        token = strtok(NULL, ":");
    }
    free(copia);
    return true;
}

bool validar_condicion_for(const char* linea, int num_linea) {
    if (!comienza_con_insensible(linea, "for ")) {
        mostrar_error("El 'for' debe comenzar con 'for'", num_linea, linea);
        return false;
    }
    if (!strstr(linea, ":=")) {
        mostrar_error("Falta ':=' en la inicialización del 'for'", num_linea, linea);
        return false;
    }
    if (!strstr(linea, " to ") && !strstr(linea, " downto ")) {
        mostrar_error("Falta 'to' o 'downto' en el 'for'", num_linea, linea);
        return false;
    }
    if (!termina_con(linea, "do")) {
        mostrar_error("El 'for' debe terminar con 'do'", num_linea, linea);
        return false;
    }
    return true;
}

char* extraer_parentesis(const char* str) {
    const char* ini = strchr(str, '(');
    const char* fin = strrchr(str, ')');
    if (!ini || !fin || ini > fin) return NULL;
    size_t len = fin - ini - 1;
    char* resultado = (char*)malloc(len + 1);
    strncpy(resultado, ini + 1, len);
    resultado[len] = '\0';
    return resultado;
}

void contenido_writeln(char* linea, char* contenido) {
    const char* ini = strchr(linea, '(');
    const char* fin = strchr(linea, ')');
    if (ini && fin && ini < fin) {
        size_t len = fin - ini - 1;
        strncpy(contenido, ini + 1, len);
        contenido[len] = '\0';
    } else {
        contenido[0] = '\0';
    }
}

void extraer_condicion_if(const char* linea, char* condicion) {
    const char* ini = strstr(linea, "if");
    if (!ini) {
        condicion[0] = '\0';
        return;
    }
    ini += 2;
    while (*ini && isspace(*ini)) ini++;
    const char* fin = strstr(ini, "then");
    if (!fin) {
        condicion[0] = '\0';
        return;
    }
    size_t len = fin - ini;
    strncpy(condicion, ini, len);
    condicion[len] = '\0';
}

void extraer_condicion_while(const char* linea, char* condicion) {
    const char* ini = strstr(linea, "while");
    if (!ini) {
        condicion[0] = '\0';
        return;
    }
    ini += 5;
    while (*ini && isspace(*ini)) ini++;
    const char* fin = strstr(ini, "do");
    if (!fin) {
        condicion[0] = '\0';
        return;
    }
    size_t len = fin - ini;
    strncpy(condicion, ini, len);
    condicion[len] = '\0';
}

void extraer_condicion_for(const char* linea, char* inicializacion, char* operador_control, char* final) {
    const char* ptr = linea + 4;
    while (*ptr && isspace(*ptr)) ptr++;
    const char* pos_asignacion = strstr(ptr, ":=");
    if (!pos_asignacion) {
        strcpy(inicializacion, "");
        return;
    }

    size_t var_len = pos_asignacion - ptr;
    strncpy(inicializacion, ptr, var_len);
    inicializacion[var_len] = '\0';
    limpiar_espacios(inicializacion);

    const char* pos_to = strstr(ptr, " to ");
    const char* pos_downto = strstr(ptr, " downto ");
    const char* pos_control = pos_to ? pos_to : pos_downto;
    if (!pos_control) return;

    strcpy(operador_control, pos_to ? "to" : "downto");

    const char* ini_val = pos_asignacion + 2;
    while (*ini_val && isspace(*ini_val)) ini_val++;

    size_t len_val = pos_control - ini_val;
    char valor_ini[50];
    strncpy(valor_ini, ini_val, len_val);
    valor_ini[len_val] = '\0';
    limpiar_espacios(valor_ini);

    sprintf(inicializacion, "%s := %s", inicializacion, valor_ini);

    const char* ptr2 = pos_control + strlen(operador_control) + 1;
    while (*ptr2 && isspace(*ptr2)) ptr2++;
    const char* pos_do = strstr(ptr2, " do");
    if (pos_do) {
        size_t len_final = pos_do - ptr2;
        strncpy(final, ptr2, len_final);
        final[len_final] = '\0';
        limpiar_espacios(final);
    }
}
void obtener_nombre_funcion(const char* linea, char* nombre, size_t tam) {
    const char* ptr = linea;
    if (strncmp(ptr, "function ", 9) == 0) ptr += 9;
    while (*ptr && isspace(*ptr)) ptr++;
    size_t i = 0;
    while (*ptr && !isspace(*ptr) && *ptr != '(' && i < tam - 1) {
        nombre[i++] = *ptr++;
    }
    nombre[i] = '\0';
    convertir_minusculas(nombre);
}

void obtener_nombre_procedure(const char* linea, char* nombre, size_t tam) {
    const char* ptr = linea;
    while (*ptr && isspace(*ptr)) ptr++;
    if (strncasecmp(ptr, "procedure", 9) != 0) {
        nombre[0] = '\0';
        return;
    }
    ptr += 9;
    while (*ptr && isspace(*ptr)) ptr++;
    size_t i = 0;
    while (*ptr && !isspace(*ptr) && *ptr != '(' && *ptr != ';' && i < tam - 1) {
        nombre[i++] = *ptr++;
    }
    nombre[i] = '\0';
    convertir_minusculas(nombre);
}