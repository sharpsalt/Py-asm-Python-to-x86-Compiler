#ifndef GEN_H
#define GEN_H

#include"interpreter.h"
#include"token.h"
#include"globals.h"
#include"ast.h"

// static int genAST(struct ASTNode *n);
void generatecode(struct ASTNode *n);

#endif //GEN_H