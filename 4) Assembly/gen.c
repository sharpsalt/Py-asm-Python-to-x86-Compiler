#include<stdio.h>
#include<stdlib.h>
#include"interpreter.h"
#include"token.h"
#include"globals.h"
#include"ast.h"
#include "x86codegenerator.h"


//Given an AST
//Assembly Code Recursively
static int genAST(struct ASTNode *n){
    int leftregister,rightregister;

    //Get the left and right sub-tree Values
    if(n->left)leftregister=genAST(n->left);
    if(n->right)rightregister=genAST(n->right);

    switch (n->op)
    {
    case A_ADD:return (cgadd(leftregister,rightregister));
    case A_SUBTRACT:return (cgsub(leftregister,rightregister));
    case A_MULTIPLY:return (cgmul(leftregister,rightregister));
    case A_DIVIDE:return (cgdiv(leftregister,rightregister));
    case A_INT:return (cgload(n->intvalue));
    case A_FLOAT:return (cgload(n->floatvalue));
    default:
      fprintf(stderr, "Unknown AST operator %d\n", n->op);
      exit(1);
    }
}

void generatecode(struct ASTNode *n) {
  int reg;

  cgpreamble();
  reg= genAST(n);
  cgprintint(reg);      // Print the register with the result as an int
  cgpostamble();
}