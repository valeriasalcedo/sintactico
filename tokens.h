#ifndef TOKENS_H
#define TOKENS_H

typedef enum {
    // Palabras clave
    TOKEN_IF, TOKEN_THEN, TOKEN_ELSE, TOKEN_WHILE, TOKEN_DO,
    TOKEN_FOR, TOKEN_TO, TOKEN_BEGIN, TOKEN_END, TOKEN_VAR,
    TOKEN_INTEGER, TOKEN_BOOLEAN, TOKEN_REAL, TOKEN_PROGRAM,
    TOKEN_WRITELN,

    // Operadores
    TOKEN_ASSIGN, TOKEN_PLUS, TOKEN_MINUS, TOKEN_MUL, TOKEN_DIV,
    TOKEN_LT, TOKEN_GT, TOKEN_LE, TOKEN_GE, TOKEN_EQ, TOKEN_NEQ,

    // Delimitadores
    TOKEN_SEMICOLON, TOKEN_COLON, TOKEN_COMMA,
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_DOT,

    // Literales e identificadores
    TOKEN_ID, TOKEN_NUMBER, TOKEN_BOOL, TOKEN_STRING,

    // Finales
    TOKEN_EOF, TOKEN_ERROR
} TokenType;

#endif
