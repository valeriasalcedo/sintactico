#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Crear un nodo del AST
ASTNode* create_node(ASTNodeType type, const char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->left = NULL;
    node->right = NULL;
    node->next = NULL;
    return node;
}

// Liberar memoria del AST recursivamente
void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->next);
    if (node->value) free(node->value);
    free(node);
}

// Función auxiliar para obtener nombre legible del tipo de nodo
static const char* ast_type_to_string(ASTNodeType type) {
    switch (type) {
        case AST_PROGRAM: return "Program";
        case AST_VAR_DECL: return "VarDecl";
        case AST_ASSIGN: return "Assign";
        case AST_IF: return "If";
        case AST_WHILE: return "While";
        case AST_FOR: return "For";
        case AST_WRITELN: return "Writeln";
        case AST_BLOCK: return "Block";
        case AST_BINARY_OP: return "BinaryOp";
        case AST_LITERAL: return "Literal";
        case AST_VARIABLE: return "Variable";
        default: return "Unknown";
    }
}

// Imprimir AST de forma indentada como árbol
void print_ast(ASTNode* node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) {
        printf("  "); 
    }

    printf("-- %s", ast_type_to_string(node->type));
    if (node->value)
        printf(": %s", node->value);
    printf("\n");

    if (node->left)  print_ast(node->left, depth + 1);
    if (node->right) print_ast(node->right, depth + 1);
    if (node->next)  print_ast(node->next, depth);
}
