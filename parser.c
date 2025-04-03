#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "parser.h"
#include "lexer.h"
#include "semantico.h"

// =========================
// Árbol Sintáctico
// =========================

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
void analizar_funcion(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* nombre_funcion) {
    obtener_nombre_funcion(linea, nombre_funcion, 50);
    Nodo* nodo_funcion = crear_nodo("Funcion", nombre_funcion);
    agregar_hijo(arbol, nodo_funcion);

    char* parametros = extraer_parentesis(linea);
    if (parametros) {
        char** args = split(parametros, ";", NULL);
        for (int i = 0; args[i]; i++) {
            char** partes = split(args[i], ":", NULL);
            if (partes && partes[0] && partes[1]) {
                char** nombres = split(partes[0], ",", NULL);
                TipoDato tipo = obtener_tipo_desde_string(partes[1]);
                for (int j = 0; nombres[j]; j++) {
                    limpiar_espacios(nombres[j]);
                    agregar_parametro_funcion(nombre_funcion, nombres[j], tipo);
                    agregar_variable(nombres[j], tipo, *num_linea);
                    marcar_variable_inicializada(nombres[j]);
                }
            }
        }
    }

    char* retorno = strrchr(linea, ':');
    if (retorno) {
        retorno++;
        limpiar_espacios(retorno);
        quitar_punto_y_coma(retorno);
        TipoDato tipo_retorno = obtener_tipo_desde_string(retorno);
        agregar_funcion(nombre_funcion, tipo_retorno, *num_linea);
    }

    Nodo* cuerpo = crear_nodo("CuerpoFuncion", "");
    agregar_hijo(nodo_funcion, cuerpo);
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        limpiar_espacios(buffer);
        if (strcmp(buffer, "end;") == 0) break;

        if (comienza_con(buffer, "writeln")) {
            analizar_escribir(cuerpo, buffer, *num_linea);
        } else if (comienza_con(buffer, "if")) {
            analizar_condicional_si(cuerpo, buffer, num_linea, archivo);
        } else if (comienza_con(buffer, "while")) {
            analizar_mientras(cuerpo, buffer, num_linea, archivo);
        } else if (comienza_con(buffer, "for")) {
            analizar_para(cuerpo, buffer, num_linea, archivo);
        } else if (strstr(buffer, ":=")) {
            analizar_asignacion_variable(cuerpo, buffer, *num_linea);
        }
    }
}
void analizar_procedimiento(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo, char* nombre_procedimiento) {
    char buffer[256];
    char copia_linea[256];
    strcpy(copia_linea, linea);

    limpiar_espacios(copia_linea);
    obtener_nombre_procedure(copia_linea, nombre_procedimiento, sizeof(nombre_procedimiento));

    if (strncmp(copia_linea, "procedure ", 10) != 0) {
        mostrar_error("La declaración debe comenzar con 'procedure'", *num_linea, copia_linea);
    }

    if (!termina_con(copia_linea, ";")) {
        mostrar_error("La declaración del procedimiento debe terminar con ';'", *num_linea, copia_linea);
    }

    Nodo* nodo_procedimiento = crear_nodo("Procedimiento", nombre_procedimiento);
    agregar_hijo(arbol, nodo_procedimiento);

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        limpiar_espacios(buffer);
        if (buffer[0] == '\0') continue;

        if (contiene_palabra_reservada(buffer, PALABRAS_RESERVADAS, CANTIDAD_PALABRAS_RESERVADAS)) {
            analizar_palabra_reservada(nodo_procedimiento, buffer, num_linea, true, archivo);
        }

        if (strstr(buffer, "writeln") != NULL) {
            analizar_escribir(nodo_procedimiento, buffer, *num_linea);
        }

        if (strcmp(buffer, "end;") == 0) {
            break;
        }
    }
}
void analizar_variables(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* ultima_linea) {
    char buffer[256];
    limpiar_espacios(linea);
    Nodo* nodo_var = crear_nodo("DeclaracionVariables", linea);
    agregar_hijo(arbol, nodo_var);

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        limpiar_espacios(buffer);

        if (buffer[0] == '\0') continue;

        if (comienza_con(buffer, "begin") ||
            comienza_con(buffer, "procedure") ||
            comienza_con(buffer, "function") ||
            comienza_con(buffer, "writeln")) {
            strcpy(ultima_linea, buffer);
            break;
        }

        if (!termina_con(buffer, ";")) {
            mostrar_error("Falta punto y coma ';' en declaración de variable", *num_linea, buffer);
        }

        quitar_punto_y_coma(buffer);
        char** partes = split(buffer, ":", NULL);
        if (!partes || !partes[0] || !partes[1]) {
            mostrar_error("Declaración de variable mal formada", *num_linea, buffer);
        }

        char** nombres = split(partes[0], ",", NULL);
        TipoDato tipo = obtener_tipo_desde_string(partes[1]);

        if (nombres[0] == NULL || strlen(nombres[0]) == 0) {
            mostrar_error("No se han declarado variables", *num_linea, buffer);
        }

        Nodo* nodo_asignacion = crear_nodo("AsignacionTipo", buffer);
        agregar_hijo(nodo_var, nodo_asignacion);
        Nodo* nodo_lista = crear_nodo("Variables", partes[0]);
        agregar_hijo(nodo_asignacion, nodo_lista);

        for (int i = 0; nombres[i] != NULL; i++) {
            limpiar_espacios(nombres[i]);
            if (variable_existe(nombres[i])) {
                mostrar_error("Variable ya declarada", *num_linea, nombres[i]);
            }
            agregar_variable(nombres[i], tipo, *num_linea);
            Nodo* nodo_var_indiv = crear_nodo("Variable", nombres[i]);
            agregar_hijo(nodo_lista, nodo_var_indiv);
            if (nombres[i + 1]) {
                Nodo* coma = crear_nodo("Coma", ",");
                agregar_hijo(nodo_lista, coma);
            }
        }

        Nodo* nodo_dos_puntos = crear_nodo("DosPuntos", ":");
        Nodo* nodo_tipo = crear_nodo("Tipo", partes[1]);
        agregar_hijo(nodo_asignacion, nodo_dos_puntos);
        agregar_hijo(nodo_asignacion, nodo_tipo);
    }
}
void analizar_condicional_si(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo) {
    char condicion[256];
    char buffer[256];
    extraer_condicion_if(linea, condicion);
    limpiar_espacios((char*)linea);

    if (strstr(linea, "then") != NULL && !comienza_con(linea, "if")) {
        mostrar_error("'then' debe ir después de 'if'", *num_linea, linea);
    }

    if (!termina_con(linea, "then")) {
        mostrar_error("La estructura 'if' debe terminar con 'then'", *num_linea, linea);
    }

    if (!validar_condicion(condicion, *num_linea)) {
        return;
    }

    Nodo* nodo_si = crear_nodo("IfStatement", linea);
    agregar_hijo(arbol, nodo_si);

    Nodo* nodo_if = crear_nodo("If", "if");
    agregar_hijo(nodo_si, nodo_if);

    Nodo* nodo_condicion = crear_nodo("Condicion", condicion);
    agregar_hijo(nodo_if, nodo_condicion);

    int cantidad_op = CANTIDAD_OPERADORES_COMPARACION;
    for (int i = 0; i < cantidad_op; i++) {
        if (strstr(condicion, OPERADORES_COMPARACION[i]) != NULL) {
            char** partes = split(condicion, OPERADORES_COMPARACION[i], NULL);
            Nodo* nodo_izq = crear_nodo("OperandoIzquierdo", partes[0]);
            Nodo* nodo_op = crear_nodo("Operador", OPERADORES_COMPARACION[i]);
            Nodo* nodo_der = crear_nodo("OperandoDerecho", partes[1]);

            agregar_hijo(nodo_condicion, nodo_izq);
            agregar_hijo(nodo_condicion, nodo_op);
            agregar_hijo(nodo_condicion, nodo_der);
            break;
        }
    }

    Nodo* nodo_then = crear_nodo("Then", "then");
    agregar_hijo(nodo_si, nodo_then);

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        limpiar_espacios(buffer);
        if (buffer[0] == '\0') continue;

        Nodo* nodo_sentencia = crear_nodo("Sentencia", "");
        agregar_hijo(nodo_si, nodo_sentencia);

        if (comienza_con(buffer, "writeln")) {
            analizar_escribir(nodo_sentencia, buffer, *num_linea);
        } else if (comienza_con(buffer, "if")) {
            analizar_condicional_si(nodo_sentencia, buffer, num_linea, archivo);
        } else if (comienza_con(buffer, "while")) {
            analizar_mientras(nodo_sentencia, buffer, num_linea, archivo);
        } else if (comienza_con(buffer, "for")) {
            analizar_para(nodo_sentencia, buffer, num_linea, archivo);
        } else if (strstr(buffer, ":=")) {
            analizar_asignacion_variable(nodo_sentencia, buffer, *num_linea);
        } else if (comienza_con(buffer, "end")) {
            break;
        }
    }
}
void analizar_mientras(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo) {
    char condicion[256];
    char buffer[256];
    extraer_condicion_while(linea, condicion);
    limpiar_espacios((char*)linea);

    if (!validar_condicion(condicion, *num_linea)) {
        return;
    }

    if (!termina_con(linea, "do")) {
        mostrar_error("La estructura 'while' debe terminar con 'do'", *num_linea, linea);
    }

    Nodo* nodo_mientras = crear_nodo("WhileStatement", linea);
    agregar_hijo(arbol, nodo_mientras);

    Nodo* nodo_while = crear_nodo("While", "while");
    agregar_hijo(nodo_mientras, nodo_while);

    Nodo* nodo_condicion = crear_nodo("Condicion", condicion);
    agregar_hijo(nodo_while, nodo_condicion);

    int cantidad_op = CANTIDAD_OPERADORES_COMPARACION;
    for (int i = 0; i < cantidad_op; i++) {
        if (strstr(condicion, OPERADORES_COMPARACION[i]) != NULL) {
            char** partes = split(condicion, OPERADORES_COMPARACION[i], NULL);
            Nodo* nodo_izq = crear_nodo("OperandoIzquierdo", partes[0]);
            Nodo* nodo_op = crear_nodo("Operador", OPERADORES_COMPARACION[i]);
            Nodo* nodo_der = crear_nodo("OperandoDerecho", partes[1]);

            agregar_hijo(nodo_condicion, nodo_izq);
            agregar_hijo(nodo_condicion, nodo_op);
            agregar_hijo(nodo_condicion, nodo_der);
            break;
        }
    }

    Nodo* nodo_do = crear_nodo("Do", "do");
    agregar_hijo(nodo_mientras, nodo_do);

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        limpiar_espacios(buffer);
        if (buffer[0] == '\0') continue;

        Nodo* nodo_sentencia = crear_nodo("Sentencia", "");
        agregar_hijo(nodo_mientras, nodo_sentencia);

        if (comienza_con(buffer, "writeln")) {
            analizar_escribir(nodo_sentencia, buffer, *num_linea);
        } else if (comienza_con(buffer, "if")) {
            analizar_condicional_si(nodo_sentencia, buffer, num_linea, archivo);
        } else if (comienza_con(buffer, "for")) {
            analizar_para(nodo_sentencia, buffer, num_linea, archivo);
        } else if (strstr(buffer, ":=")) {
            analizar_asignacion_variable(nodo_sentencia, buffer, *num_linea);
        } else if (comienza_con(buffer, "end")) {
            break;
        }
    }
}
void analizar_para(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo) {
    char buffer[256];
    char inicializacion[256];
    char operador_control[10];
    char valor_final[50];

    extraer_condicion_for(linea, inicializacion, operador_control, valor_final);

    if (!variable_existe(strtok(inicializacion, ":"))) {
        mostrar_error("Variable no declarada en estructura 'for'", *num_linea, linea);
    }

    if (!validar_condicion_for(linea, *num_linea)) {
        return;
    }

    Nodo* nodo_para = crear_nodo("ForStatement", linea);
    agregar_hijo(arbol, nodo_para);

    Nodo* nodo_for = crear_nodo("For", "for");
    agregar_hijo(nodo_para, nodo_for);

    analizar_asignacion_variable(nodo_para, inicializacion, *num_linea);

    Nodo* nodo_operador = crear_nodo("OperadorControl", operador_control);
    Nodo* nodo_final = crear_nodo("ValorFinal", valor_final);
    agregar_hijo(nodo_para, nodo_operador);
    agregar_hijo(nodo_para, nodo_final);

    Nodo* nodo_do = crear_nodo("Do", "do");
    agregar_hijo(nodo_para, nodo_do);

    bool tiene_bloque = false;
    long pos = ftell(archivo);

    if (fgets(buffer, sizeof(buffer), archivo)) {
        limpiar_espacios(buffer);
        if (strcmp(buffer, "begin") == 0) {
            tiene_bloque = true;
            (*num_linea)++;
            analizar_palabra_reservada(nodo_para, buffer, num_linea, false, archivo);
        } else {
            fseek(archivo, pos, SEEK_SET);
        }
    }

    if (!tiene_bloque) {
        if (fgets(buffer, sizeof(buffer), archivo)) {
            (*num_linea)++;
            limpiar_espacios(buffer);

            Nodo* nodo_sentencia = crear_nodo("Sentencia", "");
            agregar_hijo(nodo_para, nodo_sentencia);

            if (comienza_con(buffer, "writeln")) {
                analizar_escribir(nodo_sentencia, buffer, *num_linea);
            } else if (strstr(buffer, ":=")) {
                analizar_asignacion_variable(nodo_sentencia, buffer, *num_linea);
            } else if (es_llamada_funcion(buffer)) {
                procesar_llamada_funcion(buffer, *num_linea);
            }
        }
    }
}
void analizar_escribir(Nodo* arbol, const char* linea, int num_linea) {
    char contenido[256];
    contenido_writeln((char*)linea, contenido);
    limpiar_espacios((char*)linea);

    if (!termina_con(linea, ";")) {
        mostrar_error("La sentencia writeln debe terminar con ';'", num_linea, linea);
    }

    if (strstr(linea, "writel") != NULL && strstr(linea, "writeln") == NULL) {
        mostrar_error("¿Quiso escribir 'writeln'?", num_linea, linea);
        return;
    }

    Nodo* nodo_writeln = crear_nodo("Writeln", linea);
    agregar_hijo(arbol, nodo_writeln);

    char* argumento = extraer_parentesis(linea);
    if (!argumento) {
        mostrar_error("Paréntesis faltante en writeln", num_linea, linea);
        return;
    }

    size_t len = strlen(argumento);
    if ((argumento[0] == '\'' && argumento[len - 1] != '\'') ||
        (argumento[len - 1] == '\'' && argumento[0] != '\'')) {
        mostrar_error("Comilla simple faltante en writeln", num_linea, linea);
    }

    if (argumento[0] != '\'' && argumento[len - 1] != '\'') {
        for (size_t i = 1; i < len - 1; i++) {
            if (isspace(argumento[i])) {
                mostrar_error("Espacios no permitidos fuera de comillas en writeln", num_linea, argumento);
            }
        }
    }

    Nodo* nodo_contenido = crear_nodo("Contenido", argumento);
    agregar_hijo(nodo_writeln, nodo_contenido);
}
void analizar_asignacion_variable(Nodo* arbol, const char* linea, int num_linea) {
    if (!validar_asignacion(linea, num_linea)) return;

    int count = 0;
    char** partes = split(linea, ":=", &count);

    if (!partes || count != 2) {
        mostrar_error("Asignación mal formada", num_linea, linea);
        return;
    }

    limpiar_espacios(partes[0]);
    limpiar_espacios(partes[1]);

    char var_izq[256], expr_der[256];
    strcpy(var_izq, partes[0]);
    strcpy(expr_der, partes[1]);

    convertir_minusculas(var_izq);
    convertir_minusculas(expr_der);

    bool var_existe = variable_existe(var_izq);
    bool func_existe = funcion_existe(var_izq);

    if (!var_existe && !func_existe) {
        mostrar_error("Variable o función no declarada", num_linea, partes[0]);
    }

    TipoDato tipo_izq;
    if (var_existe) {
        Variable* var = buscar_variable(var_izq);
        tipo_izq = var->tipo;
        marcar_variable_inicializada(var_izq);
    } else {
        Funcion* func = buscar_funcion(var_izq);
        tipo_izq = func->tipo_retorno;
        marcar_funcion_con_retorno(var_izq);
    }

    if (es_llamada_funcion(expr_der)) {
        procesar_llamada_funcion(expr_der, num_linea);
    }

    TipoDato tipo_der = inferir_tipo_expresion(expr_der);

    if (!verificar_tipos_compatibles(tipo_izq, tipo_der)) {
        char mensaje[100];
        sprintf(mensaje, "Tipos incompatibles en asignación. Esperado %d, recibido %d", tipo_izq, tipo_der);
        mostrar_error(mensaje, num_linea, linea);
    }

    Nodo* nodo_asignacion = crear_nodo("Asignacion", "");
    agregar_hijo(arbol, nodo_asignacion);

    Nodo* nodo_var = crear_nodo("Variable", var_izq);
    Nodo* nodo_op = crear_nodo("Operador", ":=");
    Nodo* nodo_expr = crear_nodo("Expresion", expr_der);

    agregar_hijo(nodo_asignacion, nodo_var);
    agregar_hijo(nodo_asignacion, nodo_op);
    agregar_hijo(nodo_asignacion, nodo_expr);

    if (strlen(expr_der) > 1) {
        analizar_expresion(nodo_expr, expr_der, &num_linea);

    }

    for (int i = 0; i < count; i++) {
        free(partes[i]);
    }
    free(partes);
}
void analizar_palabra_reservada(Nodo* arbol, const char* linea, int* num_linea, bool desde_funcion_o_procedimiento, FILE* archivo) {
    char buffer[256];
    char copia_linea[256];
    strcpy(copia_linea, linea);
    limpiar_espacios(copia_linea);

    char primera[256] = {0};
    sscanf(copia_linea, "%s", primera);
    quitar_punto_y_coma(primera);

    if (strncmp(primera, "end", 3) == 0) {
        if (strlen(linea) > 3 && (linea[strlen(linea) - 1] == '.' || linea[strlen(linea) - 1] == ';')) {
            Nodo* nodo_end = crear_nodo("PalabraClave", linea);
            agregar_hijo(arbol, nodo_end);
            return;
        }
    }

    if (sugerencia_palabra_reservada(primera, *num_linea)) return;

    bool es_valida = false;
    int total = CANTIDAD_PALABRAS_RESERVADAS;
    for (int i = 0; i < total; i++) {
        if (strcmp(primera, PALABRAS_RESERVADAS[i]) == 0) {
            es_valida = true;
            break;
        }
    }

    if (!es_valida) {
        mostrar_error("Palabra clave no reconocida", *num_linea, primera);
        return;
    }

    if (strcmp(primera, "begin") == 0) {
        Nodo* nodo_begin = crear_nodo("Begin", linea);
        agregar_hijo(arbol, nodo_begin);

        while (fgets(buffer, sizeof(buffer), archivo)) {
            (*num_linea)++;
            limpiar_espacios(buffer);
            if (buffer[0] == '\0') continue;

            if (comienza_con(buffer, "if")) {
                analizar_condicional_si(nodo_begin, buffer, num_linea, archivo);
            } else if (comienza_con(buffer, "while")) {
                analizar_mientras(nodo_begin, buffer, num_linea, archivo);
            } else if (comienza_con(buffer, "writeln")) {
                analizar_escribir(nodo_begin, buffer, *num_linea);
            } else if (comienza_con(buffer, "for")) {
                analizar_para(nodo_begin, buffer, num_linea, archivo);
            } else if (comienza_con(buffer, "end")) {
                Nodo* nodo_end = crear_nodo("End", buffer);
                agregar_hijo(nodo_begin, nodo_end);
                break;
            } else if (strstr(buffer, ":=")) {
                analizar_asignacion_variable(nodo_begin, buffer, *num_linea);
            } else if (es_llamada_funcion(buffer)) {
                procesar_llamada_funcion(buffer, *num_linea);
            }
        }
    } else {
        Nodo* nodo_general = crear_nodo("PalabraClave", linea);
        agregar_hijo(arbol, nodo_general);
    }
}
bool es_llamada_funcion(const char* expr) {
    return strchr(expr, '(') && strchr(expr, ')');
}

char* extraer_argumentos_funcion(const char* str) {
    const char* inicio = strchr(str, '(');
    const char* fin = strrchr(str, ')');
    if (!inicio || !fin || fin <= inicio) return NULL;

    size_t len = fin - inicio - 1;
    char* resultado = (char*)malloc(len + 1);
    strncpy(resultado, inicio + 1, len);
    resultado[len] = '\0';
    return resultado;
}

void analizar_expresion(Nodo* nodo, const char* expresion, int* num_linea) {
    Nodo* nodo_expr = crear_nodo("ExpresionSimple", expresion);
    agregar_hijo(nodo, nodo_expr);
}
