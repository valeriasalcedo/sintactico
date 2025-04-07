#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "semantico.h"

void analizar_palabra_clave(Nodo* arbol, const char* linea, int *num_linea, bool fromF_Or_P, FILE* archivo) {
    char buffer[256]; 
    trim((char*)linea);
    char palabra_temp[256];
    strcpy(palabra_temp, linea);
    
    char first_word[256] = {0};
    sscanf(palabra_temp, "%s", first_word);
    trim_semicolon(first_word);

    if (strncmp(first_word, "end", 3) == 0) {
        if (strlen(linea) > 3) {
            char last_char = linea[strlen(linea) - 1];
            if (last_char == '.' || last_char == ';') {
                Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
                agregar_hijo(arbol, nodo_keyword);
                return;
            }
        }
    }
   
    if (es_palabra_clave_similar(first_word, *num_linea)) {
        return;
    }

    const int num_palabras_clave = sizeof(palabras_clave) / sizeof(palabras_clave[0]);
    bool palabra_clave_valida = false;
    
    for (int i = 0; i < num_palabras_clave; i++) {
        if (strcmp(first_word, palabras_clave[i]) == 0) {
            palabra_clave_valida = true;
            break;
        }
    }
    
    if (!palabra_clave_valida) {
        mostrar_error("Palabra clave no reconocida", *num_linea, first_word);
        return;
    }

    if (strcmp(first_word, "begin") == 0) {
        Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
        agregar_hijo(arbol, nodo_keyword);
        while (fgets(buffer, sizeof(buffer), archivo)) {
            (*num_linea)++;
            trim(buffer);
            if (buffer[0] == '\0') continue;

            if (starts_with(buffer, "if")) {
                analizar_if(nodo_keyword, buffer, num_linea, archivo);
            }
            else if (starts_with(buffer, "while")) {
                analizar_while(nodo_keyword, buffer, num_linea, archivo);
            }
            else if (starts_with(buffer, "writeln")) {
                analizar_writeln(nodo_keyword, buffer, *num_linea);
            }
            else if (starts_with(buffer, "for")) {
                analizar_for(nodo_keyword, buffer, num_linea, archivo);
            }
            else if (starts_with(buffer, "end")) {
                Nodo* nodo_keyword = crear_nodo("palabra_clave", buffer);
                agregar_hijo(arbol, nodo_keyword);
                break;
            }
            else if (strstr(buffer, ":=") != NULL) {
                analizar_asignacion(nodo_keyword, buffer, *num_linea);
            }
            else if (es_llamada_funcion(buffer)) {
                procesar_llamada_funcion(buffer, *num_linea);
            }
        }
        return;
    } else {
        Nodo* nodo_keyword = crear_nodo("palabra_clave", linea);
        agregar_hijo(arbol, nodo_keyword);
    }
}



void analizar_cabecera_funcion(Nodo* arbol, char* linea, int num_linea, char* nombre_funcion) {
    obtenerNombreFuncion(linea, nombre_funcion, 50);
    char nombre_funcion_nosirve[256];
    int count = 0;
    bool semicolon = end_with_semicolon(linea);

    if (strlen(nombre_funcion) == 0) {
        mostrar_error("No se pudo extraer el nombre de la funcion", num_linea, linea);
        return;
    }
    
    printf("DEBUG: Procesando declaracion de funcion: '%s'\n", nombre_funcion);

    if (strncmp(linea, "function ", 9) != 0) {
        mostrar_error("La declaracion debe iniciar con 'function'", num_linea, linea);
    }

    char *abre_paren = strchr(linea, '(');
    char *cierra_paren = strchr(linea, ')');
    if (abre_paren == NULL || cierra_paren == NULL || cierra_paren < abre_paren) {
        mostrar_error("Error en la definicion de los parentesis", num_linea, linea);
    }

    char *dos_puntos = strchr(cierra_paren, ':');
    if (dos_puntos == NULL) {
        mostrar_error("Se esperaba ':' despues de la lista de parametros", num_linea, linea);
    }

    if (!semicolon) {
        mostrar_error("La declaracion debe terminar con ';'", num_linea, linea);
    }

    Nodo* nodo_funcion = crear_nodo("funcion", nombre_funcion); 
    agregar_hijo(arbol, nodo_funcion);
    Nodo *nodo_cabecera_funcion = crear_nodo("cabecera", ""); 
    agregar_hijo(nodo_funcion, nodo_cabecera_funcion);
    char **partes = split_function(linea, &count);
    trim(partes[0]);
    Nodo *nodo_funcion1 = crear_nodo("header pt1", partes[0]); 
    agregar_hijo(nodo_cabecera_funcion, nodo_funcion1);
    Nodo* nodo_dos_puntos = crear_nodo("dos puntos", ":"); 
    agregar_hijo(nodo_cabecera_funcion, nodo_dos_puntos);
    Nodo* nodo_funcion2 = crear_nodo("header pt2", partes[1]); 
    agregar_hijo(nodo_cabecera_funcion, nodo_funcion2);
    
    trim(partes[1]);
    trim_semicolon(partes[1]);
    toLowerCase(partes[1]);
    TipoDato tipo_retorno = obtener_tipo_desde_string(partes[1]);
    Funcion* func = buscar_funcion(nombre_funcion);
    if (func) {
        func->tipo_retorno = tipo_retorno;
        func->linea_declaracion = num_linea;
        printf("DEBUG: Actualizada funcion '%s' con tipo %d\n", nombre_funcion, tipo_retorno);
    } else {
        agregar_funcion(nombre_funcion, tipo_retorno, num_linea);
        printf("DEBUG: Agregada funcion '%s' con tipo %d\n", nombre_funcion, tipo_retorno);
    }
    
    if (sscanf(partes[0], "function %[^;];", nombre_funcion_nosirve) == 1) {
        char *contenido_parentesis = extraer_parentesis(partes[0]);
        if (contenido_parentesis != NULL) {
            if (!validar_parametros_funcion(contenido_parentesis, num_linea)) {
                return;
            }
        }
        Nodo* nodo_parentesis1 = crear_nodo("parentesis", "'('");
        Nodo* nodo_parentesis2 = crear_nodo("parametros", contenido_parentesis);
        Nodo* nodo_parentesis3 = crear_nodo("parentesis", "')'");
        agregar_hijo(nodo_funcion1, nodo_parentesis1);
        agregar_hijo(nodo_funcion1, nodo_parentesis2);
        agregar_hijo(nodo_funcion1, nodo_parentesis3);
        if (contenido_parentesis == NULL) {
            free(partes);
            mostrar_error("Funcion mal formada", num_linea, linea);
        }
        char **params = split(contenido_parentesis, ";", &count);
        int elem = contar_elementos(params);
        for (int i = 0; i < elem; i++) {
            Nodo* nodo_parametro = crear_nodo("parametro", params[i]);
            agregar_hijo(nodo_parentesis2, nodo_parametro);
            if (i < elem - 1) {
                Nodo* nodo_coma = crear_nodo("punto y coma", ";");
                agregar_hijo(nodo_parentesis2, nodo_coma);
            }

            char **parametros = split(params[i], ":", &count);
            int count3 = contar_elementos(parametros);
            char **paramsSameType = split(parametros[0], ",", &count);
            int numParamsSameType = contar_elementos(paramsSameType);
            
            trim(parametros[1]);
            toLowerCase(parametros[1]);
            TipoDato tipo_param = obtener_tipo_desde_string(parametros[1]);
            
            for (int k = 0; k < numParamsSameType; k++) {
                trim(paramsSameType[k]);
                agregar_parametro_funcion(nombre_funcion, paramsSameType[k], tipo_param);
                agregar_variable(paramsSameType[k], tipo_param, num_linea);
                marcar_variable_inicializada(paramsSameType[k]); 
            }
            
            for (int j = 0; j < count3; j++) {
                Nodo* nodo_param_inner = crear_nodo("parametro_inner", parametros[j]);
                agregar_hijo(nodo_parametro, nodo_param_inner);
                if (j < count3 - 1) {
                    Nodo* nodo_dos_puntos = crear_nodo("dos puntos", ":");
                    agregar_hijo(nodo_parametro, nodo_dos_puntos);
                    if (numParamsSameType > 1) {
                        for (int k = 0; k < numParamsSameType; k++) {
                            Nodo* nodo_param_inner2 = crear_nodo("parametro_inner", paramsSameType[k]);
                            agregar_hijo(nodo_param_inner, nodo_param_inner2);
                            if (k < numParamsSameType - 1) {
                                Nodo* nodo_coma = crear_nodo("coma", ",");
                                agregar_hijo(nodo_param_inner, nodo_coma);
                            }
                        }
                    }
                }
                trim(parametros[1]);
                toLowerCase(parametros[1]);
                int chars = strlen(parametros[1]);
                int isValidType = es_tipo_valido(parametros[1]);
                if (!isValidType) {
                    mostrar_error("Tipo de dato no valido", num_linea, linea);
                }
            }
            trim(partes[1]);
            trim_semicolon(partes[1]);
            toLowerCase(partes[1]);
            int isValidType = es_tipo_valido(partes[1]);
            if (!isValidType) {
                mostrar_error("Tipo de retorno de la funcion dato no valido", num_linea, linea);
            }
            free(parametros);
        }
        free(partes);
        free(contenido_parentesis);
        free(params);
    } else {
        mostrar_error("Expresion ilegal", num_linea, linea);
    }
}

void analizar_funcion(Nodo* arbol, char* linea, int* num_linea, FILE* archivo, char* nombre_funcion) {
    char buffer[256]; 
    bool cabecera_analizada = false; 
    int count = 0;
    if(!cabecera_analizada) {
        analizar_cabecera_funcion(arbol, linea, *num_linea, nombre_funcion);
        cabecera_analizada = true;
    }
    Nodo* nodo_cuerpo_funcion = crear_nodo("cuerpo_funcion", "");
    agregar_hijo(arbol, nodo_cuerpo_funcion);
    
    char ambito_anterior[50];
    strcpy(ambito_anterior, tabla.ambito_actual);
    strcpy(tabla.ambito_actual, nombre_funcion);
    
    bool retorno_encontrado = false;
    Funcion* func = buscar_funcion(nombre_funcion);
    TipoDato tipo_retorno = func ? func->tipo_retorno : TIPO_DESCONOCIDO;

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        trim(buffer);
        if(buffer[0] == '\0'){
            continue;
        }

        if(strstr(buffer, ":=") != NULL){
            char** partesRetornoFuncion = split(buffer, ":=", &count);
            if (count >= 2) {
                trim(partesRetornoFuncion[0]);
                trim(partesRetornoFuncion[1]);

                char lowerFuncName[50];
                char lowerVarName[50];
                strcpy(lowerFuncName, nombre_funcion);
                strcpy(lowerVarName, partesRetornoFuncion[0]);
                toLowerCase(lowerFuncName);
                toLowerCase(lowerVarName);
                
                if(partesRetornoFuncion[0][0] == '\0'){
                    mostrar_error("No se ha asignado una variable al valor de retorno", *num_linea, buffer);
                }else if(partesRetornoFuncion[1][0] == '\0'){
                    mostrar_error("No se ha asignado un valor a la variable de retorno", *num_linea, buffer);
                }else if(strcmp(lowerVarName, lowerFuncName) == 0){
                    retorno_encontrado = true;
                    marcar_funcion_con_retorno(nombre_funcion);
                    
                    TipoDato tipo_valor = inferir_tipo_expresion(partesRetornoFuncion[1]);
                    if (!verificar_tipos_compatibles(tipo_retorno, tipo_valor)) {
                        char mensaje[100];
                        sprintf(mensaje, "Tipo de retorno incompatible. Se esperaba %d pero se encontro %d", 
                                tipo_retorno, tipo_valor);
                        mostrar_error(mensaje, *num_linea, buffer);
                    }
                }
                analizar_asignacion(nodo_cuerpo_funcion, buffer, *num_linea);

                for(int i = 0; i < count; i++) {
                    free(partesRetornoFuncion[i]);
                }
                free(partesRetornoFuncion);
            }
        } else if(strstr(buffer, "writeln") != NULL){
            analizar_writeln(nodo_cuerpo_funcion, buffer, *num_linea);
        } else if(contiene_palabra_clave(buffer, palabras_clave, sizeof(palabras_clave) / sizeof(palabras_clave[0]))){
            analizar_palabra_clave(nodo_cuerpo_funcion, buffer, num_linea, true, archivo);
        }
        
        if (strcmp(buffer, "end;") == 0) {
    if (!retorno_encontrado && tipo_retorno != TIPO_DESCONOCIDO) {
        char mensaje[100];
        sprintf(mensaje, "La funcion '%s' debe retornar un valor de tipo %d", nombre_funcion, tipo_retorno);
        mostrar_error(mensaje, *num_linea, nombre_funcion);
    }
  
    strcpy(tabla.ambito_actual, ambito_anterior);
            break;
        }
    }
}

void analizar_expresion(Nodo* arbol, char* expr, int num_linea) {
    removeSpaces(expr);
    trim_semicolon(expr);
    int count = strlen(expr);
    int num_operadores = sizeof(operadoresAritmeticos) / sizeof(operadoresAritmeticos[0]);
    
    char temp_expr[256];
    strcpy(temp_expr, expr);
    char *token = strtok(temp_expr, "+-*/()");

    TipoDato tipo_resultado = TIPO_DESCONOCIDO;
    bool first_operand = true;

    while (token != NULL) {
        trim(token);
        if (isalpha(token[0])) {  
            Variable* var = buscar_variable(token);
            if (var) {
                if (!var->inicializada) {
                    char mensaje[100];
                    sprintf(mensaje, "Variable '%s' utilizada antes de ser inicializada", token);
                    mostrar_advertencia(mensaje, num_linea);
                }

                if (first_operand) {
                    tipo_resultado = var->tipo;
                    first_operand = false;
                } else {
                    if (!verificar_tipos_compatibles(tipo_resultado, var->tipo)) {
                        char mensaje[100];
                        sprintf(mensaje, "Tipos incompatibles en la expresion. No se puede operar %d con %d", 
                                tipo_resultado, var->tipo);
                        mostrar_error(mensaje, num_linea, expr);
                    }
                    
                    if ((tipo_resultado == TIPO_CHAR && (var->tipo == TIPO_INTEGER || var->tipo == TIPO_REAL)) ||
                        (var->tipo == TIPO_CHAR && (tipo_resultado == TIPO_INTEGER || tipo_resultado == TIPO_REAL))) {
                        char mensaje[100];
                        sprintf(mensaje, "No se puede realizar operaciones aritmeticas entre char y tipos numericos");
                        mostrar_error(mensaje, num_linea, expr);
                    }
                }
            }
        }
        token = strtok(NULL, "+-*/()");
    }
    
    TipoDato tipo_expr = inferir_tipo_expresion(expr);
    
    for (int i = 0; i < count; i++) {
        bool es_operador = false;
        for (int j = 0; j < num_operadores; j++) {  
            int op_len = strlen(operadoresAritmeticos[j]);

            if (i + op_len <= count && strncmp(&expr[i], operadoresAritmeticos[j], op_len) == 0) {
                Nodo* nodo_operador = crear_nodo("operador", operadoresAritmeticos[j]);
                agregar_hijo(arbol, nodo_operador);
                es_operador = true;
                i += op_len - 1; 
                break;
            }
        }

        if (!es_operador) {
            char operando[2] = {expr[i], '\0'};
            Nodo* nodo_operando = crear_nodo("operando", operando);
            agregar_hijo(arbol, nodo_operando);
        }
    }
}

void analizar_llamada_funcion(const char* nombre_funcion, const char* argumentos, int num_linea) {
    Funcion* func = buscar_funcion(nombre_funcion);
    if (!func) {
        mostrar_error("Funcion no declarada", num_linea, nombre_funcion);
        return;
    }
    
    int count = 0;
    char** args = NULL;
    if (strlen(argumentos) > 0) {
        args = split(argumentos, ",", &count);
    }
    
    if (count != func->num_parametros) {
        char mensaje[100];
        sprintf(mensaje, "Numero incorrecto de argumentos. Se esperaban %d pero se encontraron %d", 
                func->num_parametros, count);
        mostrar_error(mensaje, num_linea, nombre_funcion);
    } else {
        for (int i = 0; i < count; i++) {
            trim(args[i]);
            TipoDato tipo_arg = inferir_tipo_expresion(args[i]);
            TipoDato tipo_param = func->parametros[i].tipo;
            
            if (!verificar_tipos_compatibles(tipo_param, tipo_arg)) {
                char mensaje[100];
                sprintf(mensaje, "Tipo incompatible en el argumento %d. Se esperaba %d pero se encontro %d", 
                        i+1, tipo_param, tipo_arg);
                mostrar_error(mensaje, num_linea, args[i]);
            }
        }
    }
    
    if (args) {
        for (int i = 0; i < count; i++) {
            free(args[i]);
        }
        free(args);
    }
}

bool es_llamada_funcion(const char* expr) {
    char* copia = strdup(expr);
    char* token = strtok(copia, "( \t\n");
    
    if (token) {
        trim(token);
        if (isalpha(token[0]) || token[0] == '_') {
            const char* resto = expr + strlen(token);
            while (*resto && isspace(*resto)) resto++;
            
            free(copia);
            return (*resto == '(');
        }
    }
    
    free(copia);
    return false;
}

void procesar_llamada_funcion(const char* expr, int num_linea) {
    char nombre_funcion[50] = {0};
    int i = 0;
    
    while (expr[i] && expr[i] != '(' && i < 49) {
        nombre_funcion[i] = expr[i];
        i++;
    }
    nombre_funcion[i] = '\0';
    trim(nombre_funcion);
    
    printf("DEBUG: Buscando funcion: '%s'\n", nombre_funcion);
    printf("DEBUG: Funciones disponibles: %d\n", tabla.num_funciones);
    for (int j = 0; j < tabla.num_funciones; j++) {
        printf("DEBUG: Funcion %d: '%s'\n", j, tabla.funciones[j].nombre);
    }

    char nombre_lower[50];
    strcpy(nombre_lower, nombre_funcion);
    toLowerCase(nombre_lower);
    
    Funcion* func = buscar_funcion(nombre_lower);
    if (!func) {
        char error_msg[100];
        sprintf(error_msg, "Funcion no declarada -> %s -> %s", nombre_funcion, expr);
        mostrar_error(error_msg, num_linea, expr);
        return;
    }

    char* argumentos = extraer_argumentos_funcion(expr);
    if (!argumentos) {
        mostrar_error("Error al extraer argumentos de la funcion", num_linea, expr);
        return;
    }
    
    int num_args = 0;
    if (strlen(argumentos) > 0) {
        char* args_copy = strdup(argumentos);
        char* token = strtok(args_copy, ",");
        while (token) {
            num_args++;
            token = strtok(NULL, ",");
        }
        free(args_copy);
    }

    if (num_args != func->num_parametros) {
        char error_msg[100];
        sprintf(error_msg, "Numero incorrecto de argumentos para la funcion %s. Esperados: %d, Recibidos: %d", 
                nombre_funcion, func->num_parametros, num_args);
        mostrar_error(error_msg, num_linea, expr);
        free(argumentos);
        return;
    }
    
    free(argumentos);
}

void analizar_asignacion(Nodo* arbol, const char* linea, int num_linea){
    printf("ANALIZAR ASIGNACION\n");
    printf("linea: %s\n", linea);
     if (!validar_asignacion(linea, num_linea)) {
        return;
    }
    
    
    int count = 0;
    char** partes = split(linea, ":=", &count);
    printf("Count: %i\n", count);
    printf("Partes 0 %s\n", partes[0]);
    printf("Partes 1 %s\n", partes[1]);
    
    if (!partes || count != 2) {
        mostrar_error("Asignacion mal formada", num_linea, linea);
        return;
    }

    trim(partes[0]);
    trim(partes[1]);
    
    char lowerPartes0[256];
    char lowerPartes1[256];
    strcpy(lowerPartes0, partes[0]);
    strcpy(lowerPartes1, partes[1]);
    toLowerCase(lowerPartes0);
    toLowerCase(lowerPartes1);

    bool var_exists = false;
    bool func_exists = false;
    
    for (int i = 0; i < tabla.num_variables; i++) {
        char lowerVarName[50];
        strcpy(lowerVarName, tabla.variables[i].nombre);
        toLowerCase(lowerVarName);
        if (strcmp(lowerVarName, lowerPartes0) == 0) {
            var_exists = true;
            break;
        }
    }
    
    for (int i = 0; i < tabla.num_funciones; i++) {
        char lowerFuncName[50];
        strcpy(lowerFuncName, tabla.funciones[i].nombre);
        toLowerCase(lowerFuncName);
        if (strcmp(lowerFuncName, lowerPartes0) == 0) {
            func_exists = true;
            break;
        }
    }
    
    if (!var_exists && !func_exists) {
        mostrar_error("Variable o funcion no declarada", num_linea, partes[0]);
        return;
    }

    TipoDato tipo_izquierda;
    if (variable_existe(partes[0])) {
        Variable* var = buscar_variable(partes[0]);
        tipo_izquierda = var->tipo;
        marcar_variable_inicializada(partes[0]);
    } else {
        Funcion* func = buscar_funcion(partes[0]);
        tipo_izquierda = func->tipo_retorno;
        marcar_funcion_con_retorno(partes[0]);
    }

    if (es_llamada_funcion(partes[1])) {
        char func_name[50] = {0};
        int i = 0;
        while (partes[1][i] && partes[1][i] != '(' && i < 49) {
            func_name[i] = partes[1][i];
            i++;
        }
        func_name[i] = '\0';
        trim(func_name);

        char func_name_lower[50];
        strcpy(func_name_lower, func_name);
        toLowerCase(func_name_lower);

        Funcion* func = buscar_funcion(func_name_lower);
        if (func) {
            char* argumentos = extraer_argumentos_funcion(partes[1]);
            if (!argumentos) {
                mostrar_error("Error al extraer argumentos de la funcion", num_linea, partes[1]);
                for (int i = 0; i < count; i++) {
                    free(partes[i]);
                }
                free(partes);
                return;
            }

            int num_args = 0;
            if (strlen(argumentos) > 0) {
                char* args_copy = strdup(argumentos);
                char* token = strtok(args_copy, ",");
                while (token) {
                    num_args++;
                    token = strtok(NULL, ",");
                }
                free(args_copy);
            }
       
            if (num_args != func->num_parametros) {
                char error_msg[100];
                sprintf(error_msg, "Numero incorrecto de argumentos para la funcion %s. Esperados: %d, Recibidos: %d", 
                        func_name, func->num_parametros, num_args);
                mostrar_error(error_msg, num_linea, partes[1]);
                free(argumentos);
                for (int i = 0; i < count; i++) {
                    free(partes[i]);
                }
                free(partes);
                return;
            }
            
            free(argumentos);

            if (func->tipo_retorno != TIPO_DESCONOCIDO && !func->tiene_retorno) {
                char mensaje[100];
                sprintf(mensaje, "La funcion '%s' no tiene un valor de retorno asignado", func_name);
                mostrar_error(mensaje, num_linea, partes[1]);
 
                for (int i = 0; i < count; i++) {
                    free(partes[i]);
                }
                free(partes);
                return;
            }
        }
    }

    TipoDato tipo_derecha = inferir_tipo_expresion(partes[1]);
    
    if (!verificar_tipos_compatibles(tipo_izquierda, tipo_derecha)) {
        char mensaje[100];
        sprintf(mensaje, "Tipos incompatibles en la asignacion. Se esperaba %d pero se encontro %d", 
                tipo_izquierda, tipo_derecha);
        mostrar_error(mensaje, num_linea, linea);
    }

    Nodo* nodo_asignacion = crear_nodo("asignacion", "");
    agregar_hijo(arbol, nodo_asignacion);
    
    Nodo* nodo_variable = crear_nodo("variable", partes[0]);
    Nodo* nodo_asignacion_operador = crear_nodo("asignacion_operador", ":=");
    Nodo* nodo_expresion = crear_nodo("expresion", partes[1]);

    agregar_hijo(nodo_asignacion, nodo_variable);
    agregar_hijo(nodo_asignacion, nodo_asignacion_operador);
    agregar_hijo(nodo_asignacion, nodo_expresion);
    if(strlen(partes[1]) > 1){
        analizar_expresion(nodo_expresion, partes[1], num_linea);
    }
    for (int i = 0; i < count; i++) {
        free(partes[i]);
    }
    free(partes);
}

void analizar_procedure(Nodo* arbol, const char* linea, int* num_linea, FILE* archivo, char* nombre_procedure) {
    char buffer[256]; 
	trim((char*)linea);
    obtenerNombreProcedure(linea, nombre_procedure, sizeof(nombre_procedure));
    
    if (strncmp(linea, "procedure ", 10) != 0) {
        mostrar_error("La declaracion debe iniciar con 'procedure'", *num_linea, linea);
    }

    if (!end_with_semicolon(linea)) {
        mostrar_error("La declaracion debe terminar con ';'", *num_linea, linea);
    }

    Nodo* nodo_procedure = crear_nodo("procedure", nombre_procedure);
    agregar_hijo(arbol, nodo_procedure);

    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        trim(buffer);
        
        if (buffer[0] == '\0') {
            
            continue;
        }
        if(contiene_palabra_clave(buffer, palabras_clave, sizeof(palabras_clave) / sizeof(palabras_clave[0]))){
            analizar_palabra_clave(nodo_procedure, buffer, num_linea, true, archivo);
        }

        if(strstr(buffer, "writeln") != NULL){
            analizar_writeln(nodo_procedure, buffer, *num_linea);
        }

        if(strcmp(buffer, "end;") == 0){
            break;
        }
    }
}

void analizar_writeln(Nodo* arbol, const char* linea, int num_linea) {
    char contenido[256];
    contenidoWriteln((char*)linea, contenido);
    trim((char*)linea);
    if (!end_with_semicolon(linea)) {
        mostrar_error("La declaracion debe terminar con ';'", num_linea, linea);
    }
    if (strstr(linea, "writel") != NULL && strstr(linea, "writeln") == NULL) {
        mostrar_error("Comando incorrecto. ¿Quiso escribir 'writeln'?", num_linea, linea);
        return;
    }
    Nodo* nodo_writeln = crear_nodo("writeln", linea);
    agregar_hijo(arbol, nodo_writeln);
    char* contenido_en_parentesis = extraer_parentesis(linea);
    if((starts_with(contenido_en_parentesis, "\'"))){
        printf("%s empieza con comillas simples\n", contenido_en_parentesis);
        if(!ends_with(contenido_en_parentesis, "\'")){
            printf("no termina con comillas simples\n");
            mostrar_error("Comilla simple faltante", num_linea, linea);
        }
    }
    if(ends_with(contenido_en_parentesis, "\'")){
        printf("%s termina con comillas simples\n", contenido_en_parentesis);
        if(!starts_with(contenido_en_parentesis, "\'")){
            printf("no empieza con comillas simples\n");
            mostrar_error("Comilla simple faltante", num_linea, linea);
        }
    }

    size_t len = strlen(contenido_en_parentesis);

    if (contenido[0] != '\'' ||  contenido[len - 1] != '\'') {
          int espacio_en_medio = 0;
          for (size_t i = 1; i < len - 1; i++) { 
              if (isspace(contenido[i])) {
                  espacio_en_medio = 1;
                  break;
              }
          }
          if (espacio_en_medio) {
              mostrar_error("El contenido del writeln no puede contener espacios en medio de los caracteres a menos que este encerrado entre comillas simples", num_linea, linea);
          }
      }

    if(!starts_with(contenido_en_parentesis, "\'") && !ends_with(contenido_en_parentesis, "\'")){
    }
    Nodo* contenido_writeln = crear_nodo("contenido", contenido_en_parentesis);
    agregar_hijo(nodo_writeln, contenido_writeln);
    
}

void analizar_if(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo) {
    char condicion[256];
    char buffer[256];
    extraer_condicion_if(linea, condicion);
    trim((char*)linea);
    
    if (strstr(linea, "then") != NULL && !starts_with(linea, "if")) {
        mostrar_error("'then' debe ser precedido por 'if'", *num_linea, linea);
        return;
    }

    int terminaConThen = ends_with(linea, "then");
    if (!terminaConThen) {
        mostrar_error("La estructura if debe terminar con 'then'", *num_linea, linea);
    }

    if (!validar_condicion(condicion, *num_linea)) {
        return;
    }
    Nodo* nodo_if_statement = crear_nodo("if_statement", linea);
    agregar_hijo(arbol, nodo_if_statement);
    Nodo* nodo_if = crear_nodo("if", "if");
    agregar_hijo(nodo_if_statement, nodo_if);
    Nodo* contenido_if = crear_nodo("contenido", condicion);
    agregar_hijo(nodo_if, contenido_if);
    int num_operadores = sizeof(operadoresDeComparacion) / sizeof(operadoresDeComparacion[0]);
    int count = 0;
    for (int i = 0; i < sizeof(operadoresDeComparacion) / sizeof(operadoresDeComparacion[0]); i++) {
        if (strstr(condicion, operadoresDeComparacion[i]) != NULL) {
            char** partesCondicion = split(condicion, operadoresDeComparacion[i], &count);
            Nodo* nodo_operador_izq = crear_nodo("nodo_operador_izq", partesCondicion[0]);
            Nodo* nodo_operador_der = crear_nodo("nodo_operador_der", partesCondicion[1]);
            Nodo* nodo_operador = crear_nodo("operador", operadoresDeComparacion[i]);
            agregar_hijo(contenido_if, nodo_operador_izq);
            agregar_hijo(contenido_if, nodo_operador);
            agregar_hijo(contenido_if, nodo_operador_der);
            break;
        }
    }
    Nodo* then = crear_nodo("then", "then");
    agregar_hijo(nodo_if_statement, then);
    while (fgets(buffer, sizeof(buffer), archivo)) {
        (*num_linea)++;
        trim((char*)buffer);
        if (linea[0] == '\0') {
            continue;
        }
        if (starts_with(buffer, "while") || starts_with(buffer, "for")) {
            strcpy((char*)linea, buffer);
            break;
        }
        Nodo* nodo_sentencia = crear_nodo("sentencia", "");
        agregar_hijo(nodo_if_statement, nodo_sentencia);
        if (strstr(buffer, "writeln") != NULL) {
            analizar_writeln(nodo_sentencia, buffer, *num_linea);
        }
    }
}

void analizar_while(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo){
    char condicion[256];
    char buffer[256];
    trim((char*)linea);
    extraer_condicion_while(linea, condicion);
    if (!validar_condicion(condicion, *num_linea)) {
        return;
    }
    int terminaConDo = ends_with(linea, "do");
    printf("Termina con do: %i\n", terminaConDo);
    if(!terminaConDo){
        mostrar_error("La estructura while debe terminar con 'do'", *num_linea, linea);
    }
    Nodo* while_statement = crear_nodo("while_statement", linea);
    agregar_hijo(arbol, while_statement);
    Nodo* nodo_while = crear_nodo("while", "while");
    agregar_hijo(while_statement, nodo_while);
    trim(condicion);
    Nodo* contenido_while = crear_nodo("contenido", condicion);
    agregar_hijo(nodo_while, contenido_while);
    int num_operadores = sizeof(operadoresDeComparacion) / sizeof(operadoresDeComparacion[0]);
    int count = 0;
    for(int i = 0; i < num_operadores; i++){
        if(strstr(condicion, operadoresDeComparacion[i]) != NULL){
           char** partesCondicion = split(condicion, operadoresDeComparacion[i], &count);
            Nodo* nodo_operador_izq = crear_nodo("nodo_operador_izq", partesCondicion[0]);
            Nodo* nodo_operador_der = crear_nodo("nodo_operador_der", partesCondicion[1]);
            Nodo* nodo_operador = crear_nodo("operador", operadoresDeComparacion[i]);
            agregar_hijo(contenido_while, nodo_operador_izq);
            agregar_hijo(contenido_while, nodo_operador);
            agregar_hijo(contenido_while, nodo_operador_der);
            break;
        }
    }

    Nodo* nodo_do = crear_nodo("do", "do");
    agregar_hijo(while_statement, nodo_do);
    while(fgets(buffer, sizeof(buffer), archivo)){
        (*num_linea)++;
        trim((char*)buffer);
        if(linea[0] == '\0'){
            continue;
        }
        if(starts_with(buffer, "if") || starts_with(buffer, "for") || starts_with(buffer, "while")){
            strcpy((char*)linea, buffer);
            break;
        }
        Nodo* nodo_sentencia = crear_nodo("sentencia", "");
        agregar_hijo(while_statement, nodo_sentencia);
        if(strstr(buffer, "writeln") != NULL){
            analizar_writeln(nodo_sentencia, buffer, *num_linea);
        }
    }

}



void analizar_for(Nodo* arbol, const char* linea, int *num_linea, FILE* archivo) {
    char buffer[256];
    char inicializacion[256];
    char operador_control[10];
    char final[50];
    

    const char* ptr = linea + 4; 
    while (*ptr && isspace(*ptr)) ptr++;
    
    char variable[50] = {0};
    int i = 0;
    while (*ptr && !isspace(*ptr) && *ptr != ':' && i < 49) {
        variable[i++] = *ptr++;
    }
    variable[i] = '\0';
    
    if (!variable_existe(variable)) {
        char error_msg[100];
        sprintf(error_msg, "Variable o funcion no declarada -> %s", variable);
        mostrar_error(error_msg, *num_linea, linea);
        return;
    }
    
    if (!validar_condicion_for(linea, *num_linea)) {
        return;
    }
    
    extraer_condicion_for(linea, inicializacion, operador_control, final);
    
    printf("Inicializacion del for: %s\n", inicializacion);
    printf("Operador de control: %s\n", operador_control);
    printf("Valor final: %s\n", final);
    
    
    Nodo* nodo_for_statement = crear_nodo("for_statement", linea);
    agregar_hijo(arbol, nodo_for_statement);
    
    Nodo* nodo_for = crear_nodo("for", "for");
    agregar_hijo(nodo_for_statement, nodo_for);
    
    analizar_asignacion(nodo_for_statement, inicializacion, *num_linea);
    
    Nodo* nodo_operador_control = crear_nodo("operador_control", operador_control);
    agregar_hijo(nodo_for_statement, nodo_operador_control);
    
    Nodo* nodo_final = crear_nodo("final", final);
    agregar_hijo(nodo_for_statement, nodo_final);
    
    Nodo* nodo_do = crear_nodo("do", "do");
    agregar_hijo(nodo_for_statement, nodo_do);
    
    bool has_begin_block = false;
    long pos = ftell(archivo); 
    
    if (fgets(buffer, sizeof(buffer), archivo)) {
        trim(buffer);
        if (strcmp(buffer, "begin") == 0) {
            has_begin_block = true;
            (*num_linea)++;
            analizar_palabra_clave(nodo_for_statement, buffer, num_linea, false, archivo);
        } else {
            fseek(archivo, pos, SEEK_SET); 
        }
    } else {
        fseek(archivo, pos, SEEK_SET);  
    }
    
    if (!has_begin_block) {
        if (fgets(buffer, sizeof(buffer), archivo)) {
            (*num_linea)++;
            trim(buffer);
            
            Nodo* nodo_sentencia = crear_nodo("sentencia", "");
            agregar_hijo(nodo_for_statement, nodo_sentencia);
            
            if (starts_with(buffer, "writeln")) {
                analizar_writeln(nodo_sentencia, buffer, *num_linea);
            } 
            else if (strstr(buffer, ":=") != NULL) {
                analizar_asignacion(nodo_sentencia, buffer, *num_linea);
            }
            else if (es_llamada_funcion(buffer)) {
                procesar_llamada_funcion(buffer, *num_linea);
            }
        }
    }
    
    printf("Termina con do: %d\n", 1);
}


void imprimir_arbol(Nodo* nodo, int nivel) {
    for (int i = 0; i < nivel; i++) printf("  ");  
    printf("%s(%s)\n", nodo->tipo, nodo->valor);  

    for (int i = 0; i < nodo->num_hijos; i++) {
        imprimir_arbol(nodo->hijos[i], nivel + 1); 
    }
}