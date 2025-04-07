#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef enum {
    TYPE_INTEGER,
    TYPE_REAL,
    TYPE_BOOLEAN,
    TYPE_UNKNOWN
} VarType;

typedef struct Symbol {
    char* name;
    VarType type;
    struct Symbol* next;
} Symbol;

void add_symbol(const char* name, VarType type);
VarType get_symbol_type(const char* name);
void reset_symbol_table();

#endif
