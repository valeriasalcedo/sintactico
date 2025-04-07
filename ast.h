#ifndef AST_H
#define AST_H

typedef enum {
    AST_PROGRAM,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_WRITELN,
    AST_BLOCK,
    AST_BINARY_OP,
    AST_LITERAL,
    AST_VARIABLE
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char* value; 
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* next;
} ASTNode;

ASTNode* create_node(ASTNodeType type, const char* value);
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int depth);

#endif
