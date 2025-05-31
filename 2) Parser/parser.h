#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "../Scanner/token.h"

typedef struct {
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;


void init_parser(FILE* source);
ASTNode* parse_program();
void parser_sync();

#endif
