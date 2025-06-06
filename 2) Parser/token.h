#include<stdio.h>
#include<stdbool.h>
#include<ctype.h>

#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    T_EOF, T_INT, T_FLOAT, T_STRING, T_IDENTIFIER,
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_POWER,
    T_LSHIFT, T_RSHIFT, T_BIT_AND, T_BIT_OR, T_BIT_XOR, T_BIT_NOT,
    T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE, T_ASSIGN,
    T_PLUS_EQ, T_MINUS_EQ, T_STAR_EQ, T_SLASH_EQ,
    T_LSHIFT_EQ, T_RSHIFT_EQ, T_BIT_AND_EQ, T_BIT_OR_EQ, T_BIT_XOR_EQ,
    T_LPAREN, T_RPAREN, T_LBRACKET, T_RBRACKET, T_COMMA, T_COLON,
    T_IF, T_ELSE, T_WHILE, T_FOR, T_DEF, T_RETURN,
    T_INPUT, T_INT_INPUT, T_FLOAT_INPUT, T_PRINT,
    T_NEWLINE, T_INDENT, T_DEDENT, T_UNKNOWN
} TokenType;

union Value {
    int intval;
    float floatval;
};

typedef struct {
    TokenType type;
    char* text;
    
        int intvalue;
        float floatvalue;
    
    int line;
    int column;
} Token;

enum {
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_INT, A_FLOAT
};

struct ASTNode {
    int op;
    struct ASTNode *left;
    struct ASTNode *right;
    
        int intvalue;
        float floatvalue;
    int line;
    int column;
};

void init_lexer(FILE* source);
struct ASTNode *parse(FILE *source);
Token get_next_token();
const char* token_type_to_str(TokenType type);

struct ASTNode *binexpr(int ptp);
struct ASTNode *primary(void);
int op_precedence(TokenType tok);
int arithop(TokenType tok);

bool is_operator(TokenType type);
bool is_bitwise_op(TokenType type);
bool is_comparison(TokenType type);
bool is_keyword(TokenType type);

#endif