#include "parser.h"
#include "semantic.h"
#include <stdio.h>
#include <string.h>


int main() {
    char ruta[256];
    printf("Ingrese la ruta del archivo Pascal: ");
    fgets(ruta, sizeof(ruta), stdin);
    
    // Eliminar salto de línea si existe
    ruta[strcspn(ruta, "\n")] = '\0';
    
    FILE* archivo = fopen(ruta, "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return 1;
    }



    Nodo* arbol = crear_nodo("programa", "");
    analizar_archivo(archivo, arbol);
    fclose(archivo);

    imprimir_arbol(arbol, 0);
    verificar_semantica(arbol);
    liberar_nodo(arbol);

    return 0;
}
