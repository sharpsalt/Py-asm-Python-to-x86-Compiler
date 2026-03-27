#include<stdio.h>
#include<stdlib.h>
#include"interpreter.h"
#include"token.h"
#include"globals.h"
#include"ast.h"
#include "x86codegenerator.h"

struct ASTNode *binexpr(int ptp);

static int labelnum = 1;

// Generate a unique label
int genlabel(void) {
    return labelnum++;
}

//Given an AST
//Assembly Code Recursively
static int genAST(struct ASTNode *n){
    int leftregister,rightregister;

    // Handle print statement
    if (n->op == A_PRINT) {
        leftregister = genAST(n->left);
        cgprintint(leftregister);
        return -1;  // No register to return
    }

    // Handle if statement
    if (n->op == A_IF) {
        int Lfalse = genlabel();
        int Lend = genlabel();
        
        // Generate code for condition
        leftregister = genAST(n->left);
        
        // Compare with 0 and jump if false
        cgcompare_and_jump(A_EQ, leftregister, Lfalse);
        free_register(leftregister);
        
        // Generate if body (left side of right node)
        if (n->right && n->right->left) {
            genAST(n->right->left);
        }
        
        // If there's an else part, jump over it
        if (n->right && n->right->right) {
            cgjump(Lend);
        }
        
        // False label
        cglabel(Lfalse);
        
        // Generate else body (right side of right node)
        if (n->right && n->right->right) {
            genAST(n->right->right);
        }
        
        // End label
        if (n->right && n->right->right) {
            cglabel(Lend);
        }
        
        return -1;
    }

    //Get the left and right sub-tree Values
    if(n->left) leftregister = genAST(n->left);
    if(n->right) rightregister = genAST(n->right);

    switch (n->op)
    {
    case A_ADD:return (cgadd(leftregister,rightregister));
    case A_SUBTRACT:return (cgsub(leftregister,rightregister));
    case A_MULTIPLY:return (cgmul(leftregister,rightregister));
    case A_DIVIDE:return (cgdiv(leftregister,rightregister));
    case A_EQ:return (cgequal(leftregister,rightregister));
    case A_NE:return (cgnotequal(leftregister,rightregister));
    case A_LT:return (cgless(leftregister,rightregister));
    case A_GT:return (cggreater(leftregister,rightregister));
    case A_LE:return (cglessequal(leftregister,rightregister));
    case A_GE:return (cggreaterequal(leftregister,rightregister));
    case A_INT: return (cgload(n->intvalue));
    case A_FLOAT:return (cgload(n->floatvalue));
    case A_GLUE:
        // Handle statement glue - just execute both sides
        genAST(n->left);
        if (n->right) {
            genAST(n->right);
        }
        return -1;  // No register to return
    default:
      fprintf(stderr, "Unknown AST operator %d\n", n->op);
      exit(1);
    }
}

void generatecode(struct ASTNode *n) {
    cgpreamble();
    genAST(n);
    cgpostamble();
}