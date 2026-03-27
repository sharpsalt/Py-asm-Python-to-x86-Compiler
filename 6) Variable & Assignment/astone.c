#include"token.h"
#include"tokenizer.h"
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include "ast.h"

//AST Tree Functions
struct ASTNode *mkastnode(int op,struct ASTNode *left,struct ASTNode *right,int intvalue,float floatvalue,char *identval,int line ,int column){
    struct ASTNode *p;
    p=(struct ASTNode*)malloc(sizeof(struct ASTNode));
    if(p==NULL){
        fprintf(stderr,"Unable to malloc in mkastnode()\n");
        exit(1);
    }

    //Ab initialize karwa denge basically(Now we will initialize it basically)
    p->op=op;
    p->left=left;
    p->right=right;
    p->intvalue=intvalue;
    p->floatvalue=floatvalue;
    p->ident = (identval != NULL) ? strdup(identval) : NULL;
    p->line=line;
    p->column=column;
    return p;
}

struct ASTNode *mkastleaf(int op, int intvalue, float floatvalue, char *identval, int line, int column) {
    return mkastnode(op, NULL, NULL, intvalue, floatvalue, identval, line, column);
}

struct ASTNode *mkastunary(int op, struct ASTNode *left, int intvalue, float floatvalue, char *identval, int line, int column) {
    return mkastnode(op, left, NULL, intvalue, floatvalue, identval, line, column);
}
//tree.c ko astone.c