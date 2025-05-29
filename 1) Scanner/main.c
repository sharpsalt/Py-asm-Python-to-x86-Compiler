#include <stdio.h>
#include <stdlib.h>
#include "token.h"


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    FILE *source = fopen(argv[1], "r");
    if (!source) {
        perror("Error opening file");
        return 1;
    }

    Token token;
    do {
        token = get_next_token(source);
        if (token.type == T_EOF) break;

        printf("Token: type=%d, text='%s'", token.type, token.text ? token.text : "NULL");
        if (token.type == T_INT) {
            printf(", intvalue=%d", token.intvalue);
        }
        printf("\n");

        free((void*)token.text);
    } while (token.type != T_EOF);

    fclose(source);
    return 0;
}

//To run it frst go into Scanner and the paste it
//gcc -o tokenizer main.c tokenizer.c -Wall
//./tokenizer input.txt
