#include <stdio.h>
#include "parser.h"

int main() {
    // Abre el archivo Pascal para análisis
    FILE* archivo = fopen("codigo_pascal.txt", "r");
    if (archivo == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return 1;
    }

    // Crea el nodo raíz para el AST
    Nodo* raiz = crear_nodo("programa", "");
    
    // Llama a la función que analiza el archivo y genera el AST
    analizar_archivo(archivo, raiz);

    // Cierra el archivo después de ser leído
    fclose(archivo);

    // Imprime el AST resultante
    imprimir_ast(raiz, 0);

    return 0;
}
