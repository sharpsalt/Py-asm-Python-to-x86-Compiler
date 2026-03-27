#include "token.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // For strdup
#include "globals.h"
#include "ast.h" 

struct ASTNode *binexpr(int ptp);

static void next_token(void) {
    CurrentToken = get_next_token();
}

struct ASTNode *primary(void) {
    struct ASTNode *n;

    switch (CurrentToken.type) {
        case T_INT:
            n = mkastleaf(A_INT, CurrentToken.intvalue, 0.0f, NULL, CurrentToken.line, CurrentToken.column);  // FIXED: Added NULL
            next_token();
            return n;
        
        case T_FLOAT:
            n = mkastleaf(A_FLOAT, 0, CurrentToken.floatvalue, NULL, CurrentToken.line, CurrentToken.column);  // FIXED: Added NULL
            next_token();
            return n;

        case T_IDENTIFIER:
            n = mkastleaf(A_IDENT, 0, 0.0f, strdup(CurrentToken.text), CurrentToken.line, CurrentToken.column);
            next_token();
            return n;

        case T_LPAREN:
            next_token();
            n = binexpr(0);
            if (CurrentToken.type != T_RPAREN) {
                fprintf(stderr, "Expected ')' at line %d, column %d\n", CurrentToken.line, CurrentToken.column);
                exit(1);
            }
            next_token();
            return n;

        default:
            fprintf(stderr, "Syntax error: unexpected token %s at line %d, column %d\n", 
                    token_type_to_str(CurrentToken.type), CurrentToken.line, CurrentToken.column);
            exit(1);
    }
}

int op_precedence(TokenType tok) {
    switch (tok) {
        case T_EQ:
        case T_NE:
        case T_LT:
        case T_GT:
        case T_LE:
        case T_GE:
            return 5;   // Lower precedence than arithmetic
        case T_PLUS:
        case T_MINUS:
            return 10;
        case T_STAR:
        case T_SLASH:
            return 20;
        default:
            return 0;
    }
}

int arithop(TokenType tok) {
    switch (tok) {
        // Arithmetic operators
        case T_PLUS:     return A_ADD;
        case T_MINUS:    return A_SUBTRACT;
        case T_STAR:     return A_MULTIPLY;
        case T_SLASH:    return A_DIVIDE;
        
        // Comparison operators  
        case T_EQ:       return A_EQ;
        case T_NE:       return A_NE;
        case T_LT:       return A_LT;
        case T_GT:       return A_GT;
        case T_LE:       return A_LE;
        case T_GE:       return A_GE;
        
        default:
            fprintf(stderr, "Unknown token in arithop(): %s at line %d, column %d\n", 
                    token_type_to_str(tok), CurrentToken.line, CurrentToken.column);
            exit(1);
    }
}

int compop(TokenType tok) {
    switch (tok) {
        case T_EQ:       return A_EQ;
        case T_NE:       return A_NE;
        case T_LT:       return A_LT;
        case T_GT:       return A_GT;
        case T_LE:       return A_LE;
        case T_GE:       return A_GE;
        default:
            fprintf(stderr, "Unknown token in compop(): %s at line %d, column %d\n", 
                    token_type_to_str(tok), CurrentToken.line, CurrentToken.column);
            exit(1);
    }
}

struct ASTNode *binexpr(int ptp) {
    struct ASTNode *left, *right;
    int nodetype;

    left = primary();

    TokenType tokentype = CurrentToken.type;
    int tokprec = op_precedence(tokentype);

    while (tokprec > ptp) {
        next_token();

        right = binexpr(tokprec);

        left = mkastnode(arithop(tokentype), left, right, 0, 0.0f, NULL, CurrentToken.line, CurrentToken.column);  // FIXED: Added NULL

        tokentype = CurrentToken.type;
        tokprec = op_precedence(tokentype);
    }

    return left;
}

struct ASTNode *parse(FILE *input) {
    init_lexer(input);
    next_token();
    return binexpr(0);
}