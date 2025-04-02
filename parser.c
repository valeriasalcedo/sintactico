#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

Nodo* crear_nodo(const char* tipo, const char* valor) {
    Nodo* nodo = (Nodo*)malloc(sizeof(Nodo));
    strcpy(nodo->tipo, tipo);
    strcpy(nodo->valor, valor);
    nodo->hijos = NULL;
    nodo->num_hijos = 0;
    return nodo;
}

void agregar_hijo(Nodo* padre, Nodo* hijo) {
    padre->num_hijos++;
    padre->hijos = (Nodo**)realloc(padre->hijos, padre->num_hijos * sizeof(Nodo*));
    padre->hijos[padre->num_hijos - 1] = hijo;
}

void imprimir_arbol(Nodo* nodo, int nivel) {
    for (int i = 0; i < nivel; i++) printf("  ");
    printf("%s(%s)\n", nodo->tipo, nodo->valor);

    for (int i = 0; i < nodo->num_hijos; i++) {
        imprimir_arbol(nodo->hijos[i], nivel + 1);
    }
}

void liberar_nodo(Nodo* nodo) {
    for (int i = 0; i < nodo->num_hijos; i++) {
        liberar_nodo(nodo->hijos[i]);
    }
    free(nodo->hijos);
    free(nodo);
}

void analizar_asignacion(Nodo* padre, const char* linea) {
    Nodo* nodo_asignacion = crear_nodo("asignacion", "");
    agregar_hijo(padre, nodo_asignacion);

    char* copia = strdup(linea);
    char* token = strtok(copia, ":=");
    if (token) {
        Nodo* var = crear_nodo("variable", token);
        agregar_hijo(nodo_asignacion, var);
        token = strtok(NULL, ":=");
        if (token) {
            Nodo* expr = crear_nodo("expresion", token);
            agregar_hijo(nodo_asignacion, expr);
        }
    }
    free(copia);
}

void analizar_funcion(Nodo* padre, const char* linea) {
    Nodo* funcion = crear_nodo("funcion", "");
    agregar_hijo(padre, funcion);

    char nombre[64] = "";
    const char* ptr = strchr(linea, ' ');
    if (ptr) sscanf(ptr, "%s", nombre);
    Nodo* nodo_nombre = crear_nodo("nombre", nombre);
    agregar_hijo(funcion, nodo_nombre);

    const char* par1 = strchr(linea, '(');
    const char* par2 = strchr(linea, ')');
    if (par1 && par2 && par2 > par1) {
        char parametros[128];
        strncpy(parametros, par1 + 1, par2 - par1 - 1);
        parametros[par2 - par1 - 1] = '\0';
        Nodo* nodo_param = crear_nodo("parametros", parametros);
        agregar_hijo(funcion, nodo_param);
    }

    const char* dosp = strchr(linea, ':');
    if (dosp) {
        char tipo[32];
        sscanf(dosp + 1, "%s", tipo);
        Nodo* nodo_tipo = crear_nodo("tipo_retorno", tipo);
        agregar_hijo(funcion, nodo_tipo);
    }
}

void analizar_writeln(Nodo* padre, const char* linea) {
    Nodo* writeln = crear_nodo("writeln", "");
    agregar_hijo(padre, writeln);
    const char* inicio = strchr(linea, '(');
    const char* fin = strrchr(linea, ')');
    if (inicio && fin && fin > inicio) {
        char contenido[128];
        strncpy(contenido, inicio + 1, fin - inicio - 1);
        contenido[fin - inicio - 1] = '\0';
        Nodo* nodo_contenido = crear_nodo("contenido", contenido);
        agregar_hijo(writeln, nodo_contenido);
    }
}

void analizar_if(Nodo* padre, const char* linea) {
    Nodo* nodo_if = crear_nodo("if", "");
    agregar_hijo(padre, nodo_if);
    const char* inicio = strchr(linea, 'f');
    const char* then_ptr = strstr(linea, "then");
    if (inicio && then_ptr) {
        char condicion[128];
        strncpy(condicion, inicio + 2, then_ptr - inicio - 2);
        condicion[then_ptr - inicio - 2] = '\0';
        Nodo* nodo_cond = crear_nodo("condicion", condicion);
        agregar_hijo(nodo_if, nodo_cond);
    }
}

void analizar_while(Nodo* padre, const char* linea) {
    Nodo* nodo_while = crear_nodo("while", "");
    agregar_hijo(padre, nodo_while);
    const char* do_ptr = strstr(linea, "do");
    if (do_ptr) {
        char condicion[128];
        strncpy(condicion, linea + 5, do_ptr - (linea + 5));
        condicion[do_ptr - (linea + 5)] = '\0';
        Nodo* nodo_cond = crear_nodo("condicion", condicion);
        agregar_hijo(nodo_while, nodo_cond);
    }
}

void analizar_for(Nodo* padre, const char* linea) {
    Nodo* nodo_for = crear_nodo("for", "");
    agregar_hijo(padre, nodo_for);
    const char* to_ptr = strstr(linea, "to");
    if (!to_ptr) to_ptr = strstr(linea, "downto");
    const char* do_ptr = strstr(linea, "do");
    if (to_ptr && do_ptr) {
        char inicio[64];
        strncpy(inicio, linea + 4, to_ptr - (linea + 4));
        inicio[to_ptr - (linea + 4)] = '\0';
        Nodo* nodo_ini = crear_nodo("inicio", inicio);
        agregar_hijo(nodo_for, nodo_ini);

        char final[64];
        strncpy(final, to_ptr + ((strstr(to_ptr, "to") == to_ptr) ? 2 : 6), do_ptr - to_ptr - ((strstr(to_ptr, "to") == to_ptr) ? 2 : 6));
        final[do_ptr - to_ptr - ((strstr(to_ptr, "to") == to_ptr) ? 2 : 6)] = '\0';
        Nodo* nodo_fin = crear_nodo("final", final);
        agregar_hijo(nodo_for, nodo_fin);
    }
}

void analizar_archivo(FILE* archivo, Nodo* arbol) {
    char linea[256];
    int num_linea = 0;

    while (fgets(linea, sizeof(linea), archivo)) {
        num_linea++;
        char* ptr = linea;
        while (isspace(*ptr)) ptr++;
        if (*ptr == '\0') continue;

        if (strncmp(ptr, "function", 8) == 0) {
            analizar_funcion(arbol, ptr);
        } else if (strncmp(ptr, "begin", 5) == 0) {
            Nodo* b = crear_nodo("begin", "begin");
            agregar_hijo(arbol, b);
        } else if (strncmp(ptr, "end", 3) == 0) {
            Nodo* e = crear_nodo("end", "end");
            agregar_hijo(arbol, e);
        } else if (strstr(ptr, ":=")) {
            analizar_asignacion(arbol, ptr);
        } else if (strncmp(ptr, "writeln", 7) == 0) {
            analizar_writeln(arbol, ptr);
        } else if (strncmp(ptr, "if", 2) == 0) {
            analizar_if(arbol, ptr);
        } else if (strncmp(ptr, "while", 5) == 0) {
            analizar_while(arbol, ptr);
        } else if (strncmp(ptr, "for", 3) == 0) {
            analizar_for(arbol, ptr);
        } else {
            Nodo* nodo_otro = crear_nodo("linea", ptr);
            agregar_hijo(arbol, nodo_otro);
        }
    }
}
