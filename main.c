#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "lexer.h"
#include "semantico.h"

int main() {
    FILE* archivo = fopen("codigo_pascal.txt", "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return 1;
    }

    inicializar_tabla_simbolos();

    Nodo* arbol = crear_nodo("Programa", "");
    char linea[256];
    char nombre_funcion[50];
    char nombre_procedimiento[50];
    int num_linea = 0;

    // Primera pasada: registrar funciones para permitir llamadas anticipadas
    while (fgets(linea, sizeof(linea), archivo)) {
        limpiar_espacios(linea);
        if (linea[0] == '\0') {
            num_linea++;
            continue;
        }

        char temp[256];
        strcpy(temp, linea);
        convertir_minusculas(temp);

        if (comienza_con(temp, "function")) {
            char nombre_detectado[50] = {0};
            const char* ptr = temp + 9;
            while (*ptr && isspace(*ptr)) ptr++;

            int i = 0;
            while (*ptr && !isspace(*ptr) && *ptr != '(' && i < 49) {
                nombre_detectado[i++] = *ptr++;
            }
            nombre_detectado[i] = '\0';

            char* dos_puntos = strrchr(temp, ':');
            if (dos_puntos) {
                char tipo_str[50] = {0};
                char* fin = strchr(dos_puntos, ';');
                if (fin) {
                    strncpy(tipo_str, dos_puntos + 1, fin - dos_puntos - 1);
                } else {
                    strcpy(tipo_str, dos_puntos + 1);
                }
                limpiar_espacios(tipo_str);
                convertir_minusculas(tipo_str);

                TipoDato tipo_retorno = obtener_tipo_desde_string(tipo_str);
                if (!funcion_existe(nombre_detectado)) {
                    agregar_funcion(nombre_detectado, tipo_retorno, num_linea);
                }
            }
        }

        num_linea++;
    }

    rewind(archivo);
    num_linea = 0;

    // Segunda pasada: análisis completo
    while (fgets(linea, sizeof(linea), archivo)) {
        limpiar_espacios(linea);
        char ultima_linea[256];
        if (*linea == '\0') {
            num_linea++;
            continue;
        }

        char linea_minusculas[256];
        strcpy(linea_minusculas, linea);
        convertir_minusculas(linea_minusculas);

        char temp[256];
        strcpy(temp, linea_minusculas);
        quitar_punto_y_coma(temp);
        if (sugerencia_palabra_reservada(temp, num_linea)) {
            num_linea++;
            continue;
        }

        if (comienza_con(linea_minusculas, "var")) {
            analizar_variables(arbol, linea, &num_linea, archivo, ultima_linea);
        } else if (comienza_con(linea_minusculas, "function")) {
            analizar_funcion(arbol, linea, &num_linea, archivo, nombre_funcion);
        } else if (comienza_con(linea_minusculas, "procedure")) {
            analizar_procedimiento(arbol, linea, &num_linea, archivo, nombre_procedimiento);
        } else if (comienza_con(linea_minusculas, "if")) {
            analizar_condicional_si(arbol, linea, &num_linea, archivo);
        } else if (comienza_con(linea_minusculas, "begin")) {
            analizar_palabra_reservada(arbol, linea, &num_linea, false, archivo);
        } else if (comienza_con(linea_minusculas, "while")) {
            analizar_mientras(arbol, linea, &num_linea, archivo);
        } else if (comienza_con(linea_minusculas, "for")) {
            analizar_para(arbol, linea, &num_linea, archivo);
        } else if (comienza_con(linea_minusculas, "writeln")) {
            analizar_escribir(arbol, linea, num_linea);
        }

        num_linea++;
    }

    fclose(archivo);

    printf("Árbol de análisis sintáctico-semántico:\n");
    imprimir_arbol(arbol, 0);

    return 0;
}
