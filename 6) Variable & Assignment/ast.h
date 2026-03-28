#ifndef AST_H
#define AST_H

#include "token.h"

// AST node types
enum {
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
    A_INT, A_FLOAT,
    A_IDENT, A_ASSIGN,
    A_PRINT, A_IF, A_GLUE
};

// Value types for interpreter
typedef enum {
    VT_VOID, VT_INT, VT_FLOAT
} ValueType;

// Structure for interpreter results
struct EvalResult {
    ValueType type;
    union {
        int intval;
        float floatval;
    } v;
};

// AST Node structure
struct ASTNode {
    int op;
    struct ASTNode *left;
    struct ASTNode *right;
    union {
        int intvalue;
        float floatvalue;
        char *ident; // For identifiers
    };
    int line;
    int column;
};

struct ASTNode *mkastleaf(int op, int intvalue, float floatvalue, const char *ident, int line, int column);
struct ASTNode *mkastnode(int op, struct ASTNode *left, struct ASTNode *right, 
                         int intvalue, float floatvalue, const char *ident, int line, int column);
struct ASTNode *mkastunary(int op, struct ASTNode *left, int intvalue, float floatvalue, const char *ident, int line, int column);

#endif
