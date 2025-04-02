#include "semantic.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool declarado_variable(Nodo* raiz, const char* nombre) {
    if (strcmp(raiz->tipo, "linea") == 0 && strstr(raiz->valor, ":")) {
        if (strstr(raiz->valor, nombre)) {
            return true;
        }
    }
    for (int i = 0; i < raiz->num_hijos; i++) {
        if (declarado_variable(raiz->hijos[i], nombre)) return true;
    }
    return false;
}

void buscar_uso_variables(Nodo* nodo, Nodo* raiz) {
    if (strcmp(nodo->tipo, "variable") == 0) {
        char nombre[50];
        strcpy(nombre, nodo->valor);
        char* p = nombre;
        while (*p == ' ') p++;
        p[strcspn(p, " \n\r;():")] = '\0';

        if (!declarado_variable(raiz, p)) {
            printf("Advertencia: variable no declarada -> %s\n", p);
        }
    }
    for (int i = 0; i < nodo->num_hijos; i++) {
        buscar_uso_variables(nodo->hijos[i], raiz);
    }
}

void verificar_semantica(Nodo* arbol) {
    buscar_uso_variables(arbol, arbol);
}
