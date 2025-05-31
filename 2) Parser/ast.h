#ifndef AST_H
#define AST_H

typedef enum {
    AST_INT,
    AST_IDENT,
    AST_BINOP,
    AST_ASSIGN,
    AST_UNARY
} ASTNodeType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_LT,    // <
    OP_GT,    // >
    OP_LE,    // <=
    OP_GE,    // >=
    OP_EQ,    // ==
    OP_NE     // !=
} BinOpType;

typedef enum {
    OP_NEG,   // Unary minus (-)
    OP_NOT    // Bitwise not (~)
} UnaryOpType;


typedef struct ASTNode {
    ASTNodeType type;
    union {
        int int_value;
        char *name;

        struct {
            BinOpType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;

        struct {
            char *name;
            struct ASTNode *value;
        } assign;

        struct {
            UnaryOpType op;
            struct ASTNode *expr;
        } unary;
    };
} ASTNode;

void ast_print(ASTNode *node, int depth);
void ast_free(ASTNode *node);
void print_ast(ASTNode *node);
int interpret_ast(ASTNode *node);

ASTNode *new_int_node(int value);
ASTNode *new_ident_node(char *name);
ASTNode *new_binop_node(BinOpType op, ASTNode *left, ASTNode *right);
ASTNode *new_assign_node(char *name, ASTNode *value);
ASTNode *new_unary_node(UnaryOpType op, ASTNode *expr);

#endif
