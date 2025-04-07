#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME_LEN 256

static FILE* source = NULL;
static int current_char;
static int line = 1;

void init_lexer(const char* filename) {
    source = fopen(filename, "r");
    if (!source) {
        perror("Error abriendo archivo");
        exit(1);
    }
    current_char = fgetc(source);
}

void close_lexer() {
    if (source) fclose(source);
}

static void advance() {
    current_char = fgetc(source);
    if (current_char == '\n') line++;
}

static void skip_whitespace_and_comments() {
    while (isspace(current_char) || current_char == '{') {
        if (isspace(current_char)) {
            advance();
        } else if (current_char == '{') {
            while (current_char != '}' && current_char != EOF) {
                advance();
            }
            advance(); 
        }
    }
}

static Token make_token(TokenType type, const char* lexeme) {
    Token token;
    token.type = type;
    token.lexeme = strdup(lexeme);
    token.line = line;
    return token;
}

static Token error_token(const char* msg) {
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme = strdup(msg);
    token.line = line;
    return token;
}

static int match(char expected) {
    if (current_char == expected) {
        advance();
        return 1;
    }
    return 0;
}

static Token identifier_or_keyword() {
    char buffer[MAX_LEXEME_LEN];
    int len = 0;

    while (isalnum(current_char) || current_char == '_') {
        if (len < MAX_LEXEME_LEN - 1)
            buffer[len++] = current_char;
        advance();
    }

    buffer[len] = '\0';

    // Palabras clave
    if (strcmp(buffer, "if") == 0) return make_token(TOKEN_IF, buffer);
    if (strcmp(buffer, "then") == 0) return make_token(TOKEN_THEN, buffer);
    if (strcmp(buffer, "else") == 0) return make_token(TOKEN_ELSE, buffer);
    if (strcmp(buffer, "while") == 0) return make_token(TOKEN_WHILE, buffer);
    if (strcmp(buffer, "do") == 0) return make_token(TOKEN_DO, buffer);
    if (strcmp(buffer, "for") == 0) return make_token(TOKEN_FOR, buffer);
    if (strcmp(buffer, "to") == 0) return make_token(TOKEN_TO, buffer);
    if (strcmp(buffer, "begin") == 0) return make_token(TOKEN_BEGIN, buffer);
    if (strcmp(buffer, "end") == 0) return make_token(TOKEN_END, buffer);
    if (strcmp(buffer, "var") == 0) return make_token(TOKEN_VAR, buffer);
    if (strcmp(buffer, "integer") == 0) return make_token(TOKEN_INTEGER, buffer);
    if (strcmp(buffer, "boolean") == 0) return make_token(TOKEN_BOOLEAN, buffer);
    if (strcmp(buffer, "real") == 0) return make_token(TOKEN_REAL, buffer);
    if (strcmp(buffer, "program") == 0) return make_token(TOKEN_PROGRAM, buffer);
    if (strcmp(buffer, "writeln") == 0) return make_token(TOKEN_WRITELN, buffer);
    if (strcmp(buffer, "true") == 0 || strcmp(buffer, "false") == 0) return make_token(TOKEN_BOOL, buffer);

    return make_token(TOKEN_ID, buffer);
}

static Token number() {
    char buffer[MAX_LEXEME_LEN];
    int len = 0;
    int has_dot = 0;

    while (isdigit(current_char) || (!has_dot && current_char == '.')) {
        if (current_char == '.') has_dot = 1;
        if (len < MAX_LEXEME_LEN - 1)
            buffer[len++] = current_char;
        advance();
    }

    buffer[len] = '\0';
    return make_token(TOKEN_NUMBER, buffer);
}

static Token string_literal() {
    char buffer[MAX_LEXEME_LEN];
    int len = 0;
    advance(); // abrir comilla

    while (current_char != '\'' && current_char != EOF && len < MAX_LEXEME_LEN - 1) {
        buffer[len++] = current_char;
        advance();
    }

    if (current_char == '\'') {
        advance(); // cerrar comilla
        buffer[len] = '\0';
        return make_token(TOKEN_STRING, buffer);
    }

    return error_token("Cadena sin cerrar");
}

Token next_token() {
    skip_whitespace_and_comments();

    if (current_char == EOF) return make_token(TOKEN_EOF, "EOF");
    if (isalpha(current_char)) return identifier_or_keyword();
    if (isdigit(current_char)) return number();
    if (current_char == '\'') return string_literal();

    switch (current_char) {
        case '+': advance(); return make_token(TOKEN_PLUS, "+");
        case '-': advance(); return make_token(TOKEN_MINUS, "-");
        case '*': advance(); return make_token(TOKEN_MUL, "*");
        case '/': advance(); return make_token(TOKEN_DIV, "/");
        case '(': advance(); return make_token(TOKEN_LPAREN, "(");
        case ')': advance(); return make_token(TOKEN_RPAREN, ")");
        case ';': advance(); return make_token(TOKEN_SEMICOLON, ";");
        case ',': advance(); return make_token(TOKEN_COMMA, ",");
        case '.': advance(); return make_token(TOKEN_DOT, ".");
        case ':':
            advance();
            if (current_char == '=') {
                advance();
                return make_token(TOKEN_ASSIGN, ":=");
            }
            return make_token(TOKEN_COLON, ":");
        case '=': advance(); return make_token(TOKEN_EQ, "=");
        case '<':
            advance();
            if (current_char == '=') { advance(); return make_token(TOKEN_LE, "<="); }
            if (current_char == '>') { advance(); return make_token(TOKEN_NEQ, "<>"); }
            return make_token(TOKEN_LT, "<");
        case '>':
            advance();
            if (current_char == '=') { advance(); return make_token(TOKEN_GE, ">="); }
            return make_token(TOKEN_GT, ">");
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "Caracter invalido '%c'", current_char);
    advance();
    return error_token(msg);
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_IF: return "IF";
        case TOKEN_THEN: return "THEN";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_DO: return "DO";
        case TOKEN_FOR: return "FOR";
        case TOKEN_TO: return "TO";
        case TOKEN_BEGIN: return "BEGIN";
        case TOKEN_END: return "END";
        case TOKEN_VAR: return "VAR";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_BOOLEAN: return "BOOLEAN";
        case TOKEN_REAL: return "REAL";
        case TOKEN_PROGRAM: return "PROGRAM";
        case TOKEN_WRITELN: return "WRITELN";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MUL: return "MUL";
        case TOKEN_DIV: return "DIV";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_LE: return "LE";
        case TOKEN_GE: return "GE";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NEQ: return "NEQ";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COLON: return "COLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_DOT: return "DOT";
        case TOKEN_ID: return "ID";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_BOOL: return "BOOL_LITERAL";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
