#ifndef STATEMENTS_H
#define STATEMENTS_H

#include <stdio.h>
#include "ast.h"

// Function declarations for statement parsing
struct ASTNode *statement(void);
struct ASTNode *print_statement(void);  
struct ASTNode *if_statement(void);
struct ASTNode *parse_elif_or_else(void);
struct ASTNode *parse_block(void);
struct ASTNode *statements(void);
struct ASTNode *parse_statements(FILE *input);

// Label generation for code generation
int genlabel(void);

#endif