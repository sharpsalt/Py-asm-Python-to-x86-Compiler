#ifndef AST_H
#define AST_H

// AST node types
enum {
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
    A_INTLIT, A_STRLIT,
    A_IDENT, A_ASSIGNVAL, A_ASSIGN,
    A_PRINT, A_IF, A_GLUE
};

// Abstract Syntax Tree structure
struct ASTnode {
    int op; // Operation to be performed on this tree
    struct ASTnode *left;
    struct ASTnode *right;
    union v {
        int intvalue; // For A_INTLIT, the integer value
        int id; // For A_IDENT, the symbol slot number
    } v;
};

struct ASTNode *mkastleaf(int op, int intvalue, float floatvalue, char *identval, int line, int column);
struct ASTNode *mkastnode(int op, struct ASTNode *left, struct ASTNode *right, 
                         int intvalue, float floatvalue, char *identval, int line, int column);
struct ASTNode *mkastunary(int op, struct ASTNode *left, int intvalue, float floatvalue, char *identval, int line, int column);

#endif
