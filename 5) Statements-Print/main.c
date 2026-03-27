#include <stdio.h>
#include "interpreter.h"
#include <stdlib.h>
#include "token.h"
#include "tokenizer.h"
#include "globals.h"
#include "ast.h"
#include "gen.h"
#include "statements.h"  

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
    
    // Open input file for parsing
    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("Error opening input file");
        return 1;
    }
    
    // Open output file for assembly
    Outfile = fopen("out.s", "w");
    if (!Outfile) {
        perror("Can't open output file");
        fclose(input);
        return 1;
    }

    printf("=== COMPILING %s ===\n", argv[1]);
    
    // Parse the input file
    printf("1. Parsing statements...\n");
    struct ASTNode *ast = parse_statements(input);
    fclose(input);

    // Interpret the AST to show results
    printf("2. Interpreting AST...\n");
    struct EvalResult result = interpretAST(ast);
    
    // Generate x86 assembly code
    printf("3. Generating x86 assembly code...\n");
    generatecode(ast);
    fclose(Outfile);
    
    // Compile the assembly with printint.c
    printf("4. Compiling assembly to executable...\n");
    int compile_result = system("gcc -o program out.s printint.c");
    if (compile_result != 0) {
        printf("Error: Failed to compile assembly code\n");
        freeAST(ast);
        return 1;
    }
    
    // Run the generated program
    printf("5. Running generated program...\n");
    printf("=== PROGRAM OUTPUT ===\n");
    int run_result = system("./program");
    printf("=== END OUTPUT ===\n");
    
    if (run_result != 0) {
        printf("Warning: Program exited with code %d\n", run_result);
    }
    
    printf("\n=== COMPILATION COMPLETE ===\n");
    printf("Assembly code: out.s\n");
    printf("Executable: program\n");
    
    freeAST(ast);
    return 0;
}