#include "token.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "ast.h" 

static void next_token(void) {
    CurrentToken = get_next_token();
}

struct ASTNode *primary(void) {
    struct ASTNode *n;

    switch (CurrentToken.type) {
        case T_INT:
            n = mkastleaf(A_INT, CurrentToken.intvalue, 0.0f, CurrentToken.line, CurrentToken.column);
            next_token();
            return n;
        
        case T_FLOAT:
            n = mkastleaf(A_FLOAT, 0, CurrentToken.floatvalue, CurrentToken.line, CurrentToken.column);
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
        case T_PLUS:     return A_ADD;
        case T_MINUS:    return A_SUBTRACT;
        case T_STAR:     return A_MULTIPLY;
        case T_SLASH:    return A_DIVIDE;
        default:
            fprintf(stderr, "Unknown token in arithop(): %s at line %d, column %d\n", 
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

        left = mkastnode(arithop(tokentype), left, right, 0, 0.0f, CurrentToken.line, CurrentToken.column);

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