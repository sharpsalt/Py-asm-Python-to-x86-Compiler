#ifndef AST_H
#define AST_H

#include "token.h"

struct ASTNode *mkastleaf(int op, int intvalue, float floatvalue, char *identval, int line, int column);  // UPDATED: Added char *identval
struct ASTNode *mkastnode(int op, struct ASTNode *left, struct ASTNode *right, 
                         int intvalue, float floatvalue, char *identval, int line, int column);  // UPDATED: Added char *identval
struct ASTNode *mkastunary(int op, struct ASTNode *left, int intvalue, float floatvalue, char *identval, int line, int column);  // UPDATED: Added char *identval

#endif