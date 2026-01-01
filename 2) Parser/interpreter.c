#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "tokenizer.h"
#include "interpreter.h"

static const char *ASTop[] = {"+", "-", "*", "/"};

union Value interpretAST(struct ASTNode *n) {
    union Value leftval, rightval, result;
    int is_float = 0;

    if (n->left) leftval = interpretAST(n->left);
    if (n->right) rightval = interpretAST(n->right);

    if (n->op == A_FLOAT || (n->left && n->left->op == A_FLOAT) || (n->right && n->right->op == A_FLOAT)) {
        is_float = 1;
    }

    // Debug output
    if (n->op == A_INT) {
        printf("int %d\n", n->intvalue);
    }
    else if (n->op == A_FLOAT) {
        printf("float %f\n", n->floatvalue);
    }
    else {
        if (is_float) {
            float lval = n->left ? (n->left->op == A_FLOAT ? leftval.floatval : (float)leftval.intval) : 0.0f;
            float rval = n->right ? (n->right->op == A_FLOAT ? rightval.floatval : (float)rightval.intval) : 0.0f;
            printf("%f %s %f\n", lval, ASTop[n->op], rval);
        }
        else {
            int lval = n->left ? leftval.intval : 0;
            int rval = n->right ? rightval.intval : 0;
            printf("%d %s %d\n", lval, ASTop[n->op], rval);
        }
    }

    switch (n->op) {
        case A_ADD:
            if (is_float) {
                result.floatval = (n->left ? (n->left->op == A_FLOAT ? leftval.floatval : leftval.intval) : 0) +
                                 (n->right ? (n->right->op == A_FLOAT ? rightval.floatval : rightval.intval) : 0);
            } else {
                result.intval = (n->left ? leftval.intval : 0) + (n->right ? rightval.intval : 0);
            }
            break;

        case A_SUBTRACT:
            if (is_float) {
                result.floatval = (n->left ? (n->left->op == A_FLOAT ? leftval.floatval : leftval.intval) : 0) -
                                 (n->right ? (n->right->op == A_FLOAT ? rightval.floatval : rightval.intval) : 0);
            } else {
                result.intval = (n->left ? leftval.intval : 0) - (n->right ? rightval.intval : 0);
            }
            break;

        case A_MULTIPLY:
            if (is_float) {
                result.floatval = (n->left ? (n->left->op == A_FLOAT ? leftval.floatval : leftval.intval) : 0) *
                                 (n->right ? (n->right->op == A_FLOAT ? rightval.floatval : rightval.intval) : 0);
            } else {
                result.intval = (n->left ? leftval.intval : 0) * (n->right ? rightval.intval : 0);
            }
            break;

        case A_DIVIDE:
            if (is_float) {
                float right = n->right ? (n->right->op == A_FLOAT ? rightval.floatval : rightval.intval) : 0;
                if (right == 0.0f) {
                    fprintf(stderr, "Division by zero at line %d\n", n->line);
                    exit(1);
                }
                result.floatval = (n->left ? (n->left->op == A_FLOAT ? leftval.floatval : leftval.intval) : 0) / right;
            } else {
                if (rightval.intval == 0) {
                    fprintf(stderr, "Division by zero at line %d\n", n->line);
                    exit(1);
                }
                result.intval = (n->left ? leftval.intval : 0) / rightval.intval;
            }
            break;

        case A_INT:
            result.intval = n->intvalue;
            break;

        case A_FLOAT:
            result.floatval = n->floatvalue;
            break;

        default:
            fprintf(stderr, "Unknown AST operator %d\n", n->op);
            exit(1);
    }

    return result;
}