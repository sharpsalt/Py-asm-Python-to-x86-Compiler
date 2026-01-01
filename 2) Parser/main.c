#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "tokenizer.h"
#include "globals.h"
#include "interpreter.h"
#include "ast.h"

void freeAST(struct ASTNode *node) {
    if (node == NULL) return;
    freeAST(node->left);
    freeAST(node->right);
    free(node);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("Error opening input file");
        return 1;
    }

    struct ASTNode *ast = parse(input);
    fclose(input);

    printf("\nEvaluating expression:\n");
    union Value result = interpretAST(ast);

    if (ast->op == A_FLOAT || 
        (ast->left && ast->left->op == A_FLOAT) || 
        (ast->right && ast->right->op == A_FLOAT)) {
        printf("\nFinal result: %f\n", result.floatval);
    } else {
        printf("\nFinal result: %d\n", result.intval);
    }

    freeAST(ast);
    return 0;
}