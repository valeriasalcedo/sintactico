#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Token current;

static void advance() {
    free(current.lexeme);
    current = next_token(); // Avanza al siguiente token
}

static int match(TokenType type) {
    if (current.type == type) {
        advance(); 
        return 1; 
    } 
    return 0;
}

static void expect(TokenType type, const char* msg) {
    if (!match(type)) {
        fprintf(stderr, "Error en línea %d: Se esperaba %s pero se encontró '%s'\n", current.line, msg, current.lexeme);
        exit(1);
    }
} 

// Prototipos
static ASTNode* parse_program();
static ASTNode* parse_block();
static ASTNode* parse_statement();
static ASTNode* parse_expression();
static ASTNode* parse_term();
static ASTNode* parse_factor();

ASTNode* parse() {
    current = next_token();
    return parse_program();
}
static ASTNode* parse_var_declarations();

static ASTNode* parse_program() {
    expect(TOKEN_PROGRAM, "program");
    expect(TOKEN_ID, "nombre del programa");
    expect(TOKEN_SEMICOLON, ";");

    ASTNode* var_section = NULL;

    if (match(TOKEN_VAR)) {
        var_section = parse_var_declarations();
    }

    ASTNode* block = parse_block();
    expect(TOKEN_DOT, "."); // Fin del programa

    // Enlaza declaraciones y bloque principal
    ASTNode* prog = create_node(AST_PROGRAM, NULL);
    prog->left = var_section; 

    ASTNode* last = prog->left;
    while (last && last->next) last = last->next;
    if (last)
        last->next = block; 
    else
        prog->left = block; 

    return prog; 
}
static ASTNode* parse_var_declarations() {
    ASTNode* head = NULL;
    ASTNode* last = NULL;

    while (current.type == TOKEN_ID) {
        // Lista de identificadores
        ASTNode* id_list = NULL;
        ASTNode* id_last = NULL;

        do {
            ASTNode* id = create_node(AST_VARIABLE, current.lexeme);
            advance();

            if (!id_list) id_list = id;
            else id_last->next = id;
            id_last = id;
        } while (match(TOKEN_COMMA));

        expect(TOKEN_COLON, ":");

        // Tipo de la declaración
        char* tipo = strdup(current.lexeme);
        advance();

        ASTNode* decl = create_node(AST_VAR_DECL, tipo);
        decl->left = id_list;

        expect(TOKEN_SEMICOLON, ";");

        if (!head) head = decl;
        else last->next = decl;
        last = decl;
    }

    return head;
}



static ASTNode* parse_block() {
    expect(TOKEN_BEGIN, "begin");

    ASTNode* head = NULL;
    ASTNode* last = NULL;

    while (current.type != TOKEN_END) {
        ASTNode* stmt = parse_statement();
        expect(TOKEN_SEMICOLON, ";");

        if (!head) head = stmt;
        else last->next = stmt;
        last = stmt;
    }

    expect(TOKEN_END, "end");

    ASTNode* block = create_node(AST_BLOCK, NULL);
    block->left = head;
    return block;
}

static ASTNode* parse_statement() {
    if (current.type == TOKEN_ID) {
        // Asignación
        char* var = strdup(current.lexeme);
        advance();
        expect(TOKEN_ASSIGN, ":=");
        ASTNode* assign = create_node(AST_ASSIGN, NULL);
        assign->left = create_node(AST_VARIABLE, var);
        assign->right = parse_expression();
        return assign;
    } else if (current.type == TOKEN_IF) {
        advance();
        ASTNode* cond = parse_expression();
        expect(TOKEN_THEN, "then");
        ASTNode* then_stmt = parse_statement();

        ASTNode* if_node = create_node(AST_IF, NULL);
        if_node->left = cond;
        if_node->right = then_stmt;

        if (match(TOKEN_ELSE)) {
            if_node->next = parse_statement();
        }

        return if_node;
    } else if (current.type == TOKEN_WHILE) {
        advance();
        ASTNode* cond = parse_expression();
        expect(TOKEN_DO, "do");
        ASTNode* body = parse_statement();

        ASTNode* node = create_node(AST_WHILE, NULL);
        node->left = cond;
        node->right = body;
        return node;
    } else if (current.type == TOKEN_WRITELN) {
        advance();
        expect(TOKEN_LPAREN, "(");
        ASTNode* expr = parse_expression();
        expect(TOKEN_RPAREN, ")");

        ASTNode* node = create_node(AST_WRITELN, NULL);
        node->left = expr;
        return node;
    } else if (current.type == TOKEN_BEGIN) {
        return parse_block();
    } else {
        fprintf(stderr, "Error: sentencia inesperada '%s'\n", current.lexeme);
        exit(1);
    }
}

static ASTNode* parse_expression() {
    ASTNode* left = parse_term();

    // Operadores relacionales
    while (
        current.type == TOKEN_EQ || current.type == TOKEN_NEQ ||
        current.type == TOKEN_LT || current.type == TOKEN_LE ||
        current.type == TOKEN_GT || current.type == TOKEN_GE
    ) {
        char* op = strdup(current.lexeme);
        advance();
        ASTNode* right = parse_term();
        ASTNode* expr = create_node(AST_BINARY_OP, op);
        expr->left = left;
        expr->right = right;
        left = expr;
    }

    return left;
}

static ASTNode* parse_term() {
    ASTNode* left = parse_factor();

    while (current.type == TOKEN_PLUS || current.type == TOKEN_MINUS) {
        char* op = strdup(current.lexeme);
        advance();
        ASTNode* right = parse_factor();
        ASTNode* node = create_node(AST_BINARY_OP, op);
        node->left = left;
        node->right = right;
        left = node;
    }

    return left;
}

static ASTNode* parse_factor() {
    ASTNode* node = NULL;

    if (current.type == TOKEN_NUMBER || current.type == TOKEN_STRING || current.type == TOKEN_BOOL) {
        node = create_node(AST_LITERAL, current.lexeme);
        advance();
    } else if (current.type == TOKEN_ID) {
        node = create_node(AST_VARIABLE, current.lexeme);
        advance();
    } else if (match(TOKEN_LPAREN)) {
        node = parse_expression();
        expect(TOKEN_RPAREN, ")");
    } else {
        fprintf(stderr, "Error: expresión no válida '%s'\n", current.lexeme);
        exit(1);
    }

    return node;
}
