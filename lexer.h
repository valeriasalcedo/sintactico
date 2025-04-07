#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"

typedef struct {
    TokenType type;
    char* lexeme;
    int line;
} Token;

void init_lexer(const char* filename);
Token next_token();
void close_lexer();
const char* token_type_to_string(TokenType type);

#endif
