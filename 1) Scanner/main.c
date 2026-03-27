#include "token.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE* input_file = fopen("input.py", "r");
    if (!input_file) {
        perror("Error opening input.py");
        return 1;
    }
    //then humlog lexer ko initlialize krdenge is File se
    init_lexer(input_file);
    //Ek token create krenge Token structut ka usseToekinize karenge
    Token token;
    do {
        token = get_next_token();
        printf("Token: %-12s Line: %-3d Col: %-3d", 
               token_type_to_str(token.type), 
               token.line, 
               token.column);
        
        if (token.text) {
            printf(" Text: '%s'", token.text);
        }
        if (token.type == T_INT) {
            printf(" Value: %d", token.intvalue);
        }
        else if (token.type == T_FLOAT) {
            printf(" Value: %f", token.floatvalue);
        }
        printf("\n");
        //Free the token text memory, somtime the compiler give bad response too...
        if (token.text) free(token.text);
    } while (token.type != T_EOF);
    fclose(input_file);
    return 0;
}