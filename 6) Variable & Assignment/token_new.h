#ifndef _TOKEN_H
#define _TOKEN_H

// Tokens
enum {
    T_EOF,
    T_PLUS, T_MINUS, T_STAR, T_SLASH,
    T_EQ, T_NE,
    T_LT, T_GT, T_LE, T_GE,
    T_INTLIT, T_SEMI, T_ASSIGN, T_IDENT,
    T_PRINT, T_IF, T_ELSE
};

// Token structure
struct token {
    int token;
    int intvalue;
};

// Forward declaration for ASTnode
struct ASTnode;
#endif
