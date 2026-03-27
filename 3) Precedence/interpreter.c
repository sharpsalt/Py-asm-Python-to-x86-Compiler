#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "tokenizer.h"
#include "interpreter.h"

static const char *ASTop[] = {"+", "-", "*", "/"};

struct EvalResult interpretAST(struct ASTNode *n) {
    struct EvalResult result;

    if (n->op == A_INT) {
        result.type = VT_INT;
        result.v.intval = n->intvalue;
        printf("int %d\n", n->intvalue);
        return result;
    }
    if (n->op == A_FLOAT) {
        result.type = VT_FLOAT;
        result.v.floatval = n->floatvalue;
        printf("float %f\n", n->floatvalue);
        return result;
    }

    struct EvalResult leftval = interpretAST(n->left);
    struct EvalResult rightval = interpretAST(n->right);

    int needs_float = leftval.type == VT_FLOAT || rightval.type == VT_FLOAT;

    if (needs_float) {
        float lval = (leftval.type == VT_FLOAT) ? leftval.v.floatval : (float)leftval.v.intval;
        float rval = (rightval.type == VT_FLOAT) ? rightval.v.floatval : (float)rightval.v.intval;
        printf("%f %s %f\n", lval, ASTop[n->op], rval);

        result.type = VT_FLOAT;
        switch(n->op) {
            case A_ADD: result.v.floatval = lval + rval; break;
            case A_SUBTRACT: result.v.floatval = lval - rval; break;
            case A_MULTIPLY: result.v.floatval = lval * rval; break;
            case A_DIVIDE:
                if (rval == 0.0f) {
                    fprintf(stderr, "Float division by zero\n");
                    exit(1);
                }
                result.v.floatval = lval / rval;
                break;
            default:
                fprintf(stderr, "Unknown float operator\n");
                exit(1);
        }
    } else {
        printf("%d %s %d\n", leftval.v.intval, ASTop[n->op], rightval.v.intval);

        result.type = VT_INT;
        switch(n->op) {
            case A_ADD: result.v.intval = leftval.v.intval + rightval.v.intval; break;
            case A_SUBTRACT: result.v.intval = leftval.v.intval - rightval.v.intval; break;
            case A_MULTIPLY: result.v.intval = leftval.v.intval * rightval.v.intval; break;
            case A_DIVIDE:
                if (rightval.v.intval == 0) {
                    fprintf(stderr, "Integer division by zero\n");
                    exit(1);
                }
                result.v.intval = leftval.v.intval / rightval.v.intval;
                break;
            default:
                fprintf(stderr, "Unknown integer operator\n");
                exit(1);
        }
    }

    return result;
}
