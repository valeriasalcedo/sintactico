#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    char filename[256];
    printf("Ingrese la ruta del archivo Pascal (.pas): ");
    scanf("%255s", filename);

    // Fase 1: análisis léxico
    init_lexer(filename);
    printf("\n==== Tokens lexicos ====\n");
    Token t;
    do {
        t = next_token();
        printf("[Linea %d] Token: %-15s (%s)\n", t.line, token_type_to_string(t.type), t.lexeme);
        free(t.lexeme);
    } while (t.type != TOKEN_EOF && t.type != TOKEN_ERROR);
    close_lexer();

    // Fase 2: análisis sintáctico y AST
    init_lexer(filename);
    ASTNode* root = parse();

    printf("\n==== Arbol de Sintaxis Abstracta ====\n");
    print_ast(root, 0);

    // Fase 3: análisis semántico
    printf("\n==== Analisis semantico ====\n");

    semantic_check(root);

    free_ast(root);
    reset_symbol_table();
    close_lexer();
    return 0;
}
