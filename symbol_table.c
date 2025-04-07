#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Symbol* table = NULL;

void add_symbol(const char* name, VarType type) {
    if (get_symbol_type(name) != TYPE_UNKNOWN) return; // evitar redefinición

    Symbol* sym = malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->type = type;
    sym->next = table;
    table = sym;
}

VarType get_symbol_type(const char* name) {
    for (Symbol* it = table; it != NULL; it = it->next) {
        if (strcmp(it->name, name) == 0) return it->type;
    }
    return TYPE_UNKNOWN;
}

void reset_symbol_table() {
    while (table) {
        Symbol* next = table->next;
        free(table->name);
        free(table);
        table = next;
    }
}
