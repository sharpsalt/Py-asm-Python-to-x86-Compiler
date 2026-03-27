#include <stdio.h>
#include "interpreter.h"
#include <stdlib.h>
#include "token.h"
#include "tokenizer.h"
#include "globals.h"
#include "ast.h"
#include "gen.h"

FILE *Outfile = NULL;

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
    Outfile = fopen("out.s", "w");  // ← global file pointer
    if (!Outfile) {
        perror("Can't open output file");
        return 1;
    }

    struct ASTNode *ast = parse(input);
    fclose(input);

    printf("\nEvaluating expression:\n");
    struct EvalResult result = interpretAST(ast);
    generatecode(ast);
    printf("\nFinal result: ");
    if(result.type == VT_FLOAT)
        printf("%f\n", result.v.floatval);
    else
        printf("%d\n", result.v.intval);
    // freeAST(ast);
    fclose(Outfile);
    fclose(input);
    return 0;
}
// It actually Implements Pratt Parsing
//Ye precendence ki liye use hota hai like there is another method which is used in parsing knows as Recursive deecent Parsing

