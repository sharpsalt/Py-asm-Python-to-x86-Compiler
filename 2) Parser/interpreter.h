#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "token.h"


union Value interpretAST(struct ASTNode *n);

#endif