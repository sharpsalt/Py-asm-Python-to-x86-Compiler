#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "tokenizer.h"
#include "interpreter.h"

static const char *ASTop[] = {"+", "-", "*", "/"};

struct EvalResult interpretAST(struct ASTNode *n) {
    struct EvalResult result;

    // Handle print statement
    if (n->op == A_PRINT) {
        struct EvalResult printval = interpretAST(n->left);
        printf("print: ");
        if (printval.type == VT_FLOAT) {
            printf("%.6f\n", printval.v.floatval);
        } else {
            printf("%d\n", printval.v.intval);
        }
        return printval;  // Return the printed value
    }

    // Handle if statement
    if (n->op == A_IF) {
        struct EvalResult condition = interpretAST(n->left);
        int is_true = (condition.type == VT_FLOAT) ? 
                     (condition.v.floatval != 0.0f) : 
                     (condition.v.intval != 0);
        
        if (is_true) {
            // Execute if body (left side of right node)
            if (n->right && n->right->left) {
                return interpretAST(n->right->left);
            }
        } else {
            // Execute else body (right side of right node)
            if (n->right && n->right->right) {
                return interpretAST(n->right->right);
            }
        }
        
        // Return default result if no body executed
        result.type = VT_INT;
        result.v.intval = 0;
        return result;
    }

    if (n->op == A_INT) {
        result.type = VT_INT;
        result.v.intval = n->intvalue;
        return result;
    }
    
    if (n->op == A_FLOAT) {
        result.type = VT_FLOAT;
        result.v.floatval = n->floatvalue;
        return result;
    }

    // Handle statement glue
    if (n->op == A_GLUE) {
        if (n->left) interpretAST(n->left);   // Execute left statement
        if (n->right) return interpretAST(n->right);  // Execute right statement and return its value
        result.type = VT_INT;
        result.v.intval = 0;
        return result;
    }

    struct EvalResult leftval = interpretAST(n->left);
    struct EvalResult rightval = interpretAST(n->right);

    int needs_float = leftval.type == VT_FLOAT || rightval.type == VT_FLOAT;

    if (needs_float) {
        float lval = (leftval.type == VT_FLOAT) ? leftval.v.floatval : (float)leftval.v.intval;
        float rval = (rightval.type == VT_FLOAT) ? rightval.v.floatval : (float)rightval.v.intval;

        switch(n->op) {
            case A_ADD: 
                result.type = VT_FLOAT;
                result.v.floatval = lval + rval; 
                break;
            case A_SUBTRACT: 
                result.type = VT_FLOAT;
                result.v.floatval = lval - rval; 
                break;
            case A_MULTIPLY: 
                result.type = VT_FLOAT;
                result.v.floatval = lval * rval; 
                break;
            case A_DIVIDE:
                if (rval == 0.0f) {
                    fprintf(stderr, "Float division by zero\n");
                    exit(1);
                }
                result.type = VT_FLOAT;
                result.v.floatval = lval / rval;
                break;
            case A_EQ:
                result.type = VT_INT;
                result.v.intval = (lval == rval) ? 1 : 0;
                break;
            case A_NE:
                result.type = VT_INT;
                result.v.intval = (lval != rval) ? 1 : 0;
                break;
            case A_LT:
                result.type = VT_INT;
                result.v.intval = (lval < rval) ? 1 : 0;
                break;
            case A_GT:
                result.type = VT_INT;
                result.v.intval = (lval > rval) ? 1 : 0;
                break;
            case A_LE:
                result.type = VT_INT;
                result.v.intval = (lval <= rval) ? 1 : 0;
                break;
            case A_GE:
                result.type = VT_INT;
                result.v.intval = (lval >= rval) ? 1 : 0;
                break;
            default:
                fprintf(stderr, "Unknown float operator\n");
                exit(1);
        }
    } else {
        switch(n->op) {
            case A_ADD: 
                result.type = VT_INT;
                result.v.intval = leftval.v.intval + rightval.v.intval; 
                break;
            case A_SUBTRACT: 
                result.type = VT_INT;
                result.v.intval = leftval.v.intval - rightval.v.intval; 
                break;
            case A_MULTIPLY: 
                result.type = VT_INT;
                result.v.intval = leftval.v.intval * rightval.v.intval; 
                break;
            case A_DIVIDE:
                if (rightval.v.intval == 0) {
                    fprintf(stderr, "Integer division by zero\n");
                    exit(1);
                }
                result.type = VT_INT;
                result.v.intval = leftval.v.intval / rightval.v.intval;
                break;
            case A_EQ:
                result.type = VT_INT;
                result.v.intval = (leftval.v.intval == rightval.v.intval) ? 1 : 0;
                break;
            case A_NE:
                result.type = VT_INT;
                result.v.intval = (leftval.v.intval != rightval.v.intval) ? 1 : 0;
                break;
            case A_LT:
                result.type = VT_INT;
                result.v.intval = (leftval.v.intval < rightval.v.intval) ? 1 : 0;
                break;
            case A_GT:
                result.type = VT_INT;
                result.v.intval = (leftval.v.intval > rightval.v.intval) ? 1 : 0;
                break;
            case A_LE:
                result.type = VT_INT;
                result.v.intval = (leftval.v.intval <= rightval.v.intval) ? 1 : 0;
                break;
            case A_GE:
                result.type = VT_INT;
                result.v.intval = (leftval.v.intval >= rightval.v.intval) ? 1 : 0;
                break;
            default:
                fprintf(stderr, "Unknown integer operator\n");
                exit(1);
        }
    }

    return result;
}