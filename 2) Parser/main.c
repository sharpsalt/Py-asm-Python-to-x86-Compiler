// #include <stdio.h>
// #include "../Scanner/tokenizer.h"
// #include "parser.h"
// #include "ast.h"

// int main() {
//     FILE *f = fopen("../Scanner/input.txt", "r");
//     if (!f) {
//         perror("input.txt");
//         return 1;
//     }

//     init_lexer(f);   // ✅ Pass FILE* here
//     ASTNode *tree = parse();
//     print_ast(tree);
    
//     printf("\nEvaluation:\n");
//     int result = interpret_ast(tree);
//     printf("Final result: %d\n", result);
    
//     ast_free(tree);
//     return 0;
// }
#include <stdio.h>
#include <stdlib.h>
#include "../Scanner/tokenizer.h"
#include "parser.h"
#include "ast.h"

int main(int argc, char *argv[]) {
    // Initialize with either file or stdin
    FILE *input = stdin;
    
    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (!input) {
            perror(argv[1]);
            return 1;
        }
    }

    // Initialize lexer and parser
    init_lexer(input);
    ASTNode *tree = parse_program();
    
    if (input != stdin) {
        fclose(input);
    }

    // Process and evaluate AST
    if (!tree) {
        fprintf(stderr, "Error: No AST generated\n");
        return 1;
    }

    print_ast(tree);
    
    printf("\nEvaluation:\n");
    int result = interpret_ast(tree);
    printf("Final result: %d\n", result);
    
    ast_free(tree);
    return 0;
}