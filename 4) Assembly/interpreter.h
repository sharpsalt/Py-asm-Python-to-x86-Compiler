// #ifndef INTERPRETER_H
// #define INTERPRETER_H

// #include "ast.h"
// #include "token.h"


// union Value interpretAST(struct ASTNode *n); 

// #endif44// interpreter.h

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "token.h"

enum ValueType { VT_INT, VT_FLOAT };

struct EvalResult {
    enum ValueType type;
    union Value v;
};

struct ASTNode; // Forward declaration

struct EvalResult interpretAST(struct ASTNode *n);

#endif
