#include "semantic.h"
#include "symbol_table.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int error_count = 0;

static void report_error(const char* msg, const char* detail) {
    fprintf(stderr, "Error semantico: %s [%s]\n", msg, detail ? detail : "detalle no disponible");
    error_count++;
}

static VarType check_expr(ASTNode* expr);

void semantic_check(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            semantic_check(node->left); 
            break;

        case AST_BLOCK:
            semantic_check(node->left); 
            break;

        case AST_VAR_DECL: {
            VarType type;
            if (strcmp(node->value, "integer") == 0)
                type = TYPE_INTEGER;
            else if (strcmp(node->value, "real") == 0)
                type = TYPE_REAL;
            else if (strcmp(node->value, "boolean") == 0)
                type = TYPE_BOOLEAN;
            else {
                report_error("Tipo desconocido en declaración", node->value);
                break;
            }

            ASTNode* id = node->left;
            while (id) {
                if (get_symbol_type(id->value) != TYPE_UNKNOWN) {
                    report_error("Variable ya declarada", id->value);
                } else {
                    add_symbol(id->value, type);
                }
                id = id->next;
            }
            break;
        }

        case AST_ASSIGN: {
            const char* var_name = node->left->value;
            VarType lhs_type = get_symbol_type(var_name);
            if (lhs_type == TYPE_UNKNOWN) {
                report_error("Variable no declarada", var_name);
                break;
            }

            VarType rhs_type = check_expr(node->right);
            if (lhs_type == rhs_type) break;

            if (lhs_type == TYPE_REAL && rhs_type == TYPE_INTEGER) break;

            report_error("Tipos incompatibles en asignación", var_name);
            break;
        }

        case AST_IF:
        case AST_WHILE: {
            VarType cond_type = check_expr(node->left);
            if (cond_type != TYPE_BOOLEAN) {
                report_error("Condición no booleana", "(if / while)");
            }
            semantic_check(node->right);
            if (node->next) semantic_check(node->next);
            break;
        }

        case AST_WRITELN:
            check_expr(node->left);
            break;

        default:
            semantic_check(node->left);
            semantic_check(node->right);
            break;
    }

    semantic_check(node->next);
}

static VarType check_expr(ASTNode* expr) {
    if (!expr) return TYPE_UNKNOWN;

    switch (expr->type) {
        case AST_LITERAL:
            if (strchr(expr->value, '.'))
                return TYPE_REAL;
            if (strcmp(expr->value, "true") == 0 || strcmp(expr->value, "false") == 0)
                return TYPE_BOOLEAN;
            return TYPE_INTEGER;

        case AST_VARIABLE: {
            VarType t = get_symbol_type(expr->value);
            if (t == TYPE_UNKNOWN)
                report_error("Uso de variable no declarada", expr->value);
            return t;
        }

        case AST_BINARY_OP: {
            VarType left = check_expr(expr->left);
            VarType right = check_expr(expr->right);
            const char* op = expr->value;

            if (strcmp(op, "=") == 0 || strcmp(op, "<") == 0 ||
                strcmp(op, "<=") == 0 || strcmp(op, ">") == 0 ||
                strcmp(op, ">=") == 0 || strcmp(op, "<>") == 0) {

                if (left != right &&
                    !(left == TYPE_INTEGER && right == TYPE_REAL) &&
                    !(left == TYPE_REAL && right == TYPE_INTEGER)) {
                    report_error("Comparación con tipos incompatibles", op);
                }

                return TYPE_BOOLEAN;
            }

            if (left == TYPE_BOOLEAN || right == TYPE_BOOLEAN) {
                report_error("Operación aritmética con booleano", op);
                return TYPE_UNKNOWN;
            }

            if (left == TYPE_REAL || right == TYPE_REAL)
                return TYPE_REAL;

            return TYPE_INTEGER;
        }

        default:
            return TYPE_UNKNOWN;
    }
}
