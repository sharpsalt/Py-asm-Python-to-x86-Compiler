#ifndef AST_H
#define AST_H

#include "token.h"

struct ASTNode *mkastleaf(int op, int intvalue, float floatvalue, int line, int column);
struct ASTNode *mkastnode(int op, struct ASTNode *left, struct ASTNode *right, 
                         int intvalue, float floatvalue, int line, int column);

#endif