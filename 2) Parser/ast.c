#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define MAX_SYMBOLS 100
static struct {
    char *name;
    int value;
} symbol_table[MAX_SYMBOLS];
static int num_symbols = 0;

ASTNode *new_int_node(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc failed");
        exit(1);
    }
    node->type = AST_INT;
    node->int_value = value;
    return node;
}

ASTNode *new_ident_node(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc failed");
        exit(1);
    }
    node->type = AST_IDENT;
    node->name = strdup(name); // Copy the string to avoid external modifications
    return node;
}

ASTNode *new_binop_node(BinOpType op, ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc failed");
        exit(1);
    }
    node->type = AST_BINOP;
    node->binop.op = op;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode *new_unary_node(UnaryOpType op, ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc failed");
        exit(1);
    }
    node->type = AST_UNARY;
    node->unary.op = op;
    node->unary.expr = expr;
    return node;
}

ASTNode *new_assign_node(char *name, ASTNode *value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc failed");
        exit(1);
    }
    node->type = AST_ASSIGN;
    node->assign.name = strdup(name); // Copy the string
    node->assign.value = value;
    return node;
}

void print_spaces(int count) {
    for (int i = 0; i < count; i++) printf(" ");
}
void print_tree(ASTNode *node, int depth) {
    if (!node) return;
    
    if (node->type == AST_BINOP) {
        print_tree(node->binop.right, depth + 1);
    }else if (node->type == AST_UNARY) {
        print_tree(node->unary.expr, depth + 1);
    }
    
    // Indentation
    for (int i = 0; i < depth; i++) 
        printf("    ");
    
    // Node value
    switch (node->type) {
        case AST_INT: 
            printf("%d\n", node->int_value); 
            break;
        case AST_BINOP: {
            const char *op;
            switch (node->binop.op) {
                case OP_ADD: op = "+"; break;
                case OP_SUB: op = "-"; break;
                case OP_MUL: op = "*"; break;
                case OP_DIV: op = "/"; break;
                case OP_LT: op = "<"; break;
                case OP_GT: op = ">"; break;
                case OP_LE: op = "<="; break;
                case OP_GE: op = ">="; break;
                case OP_EQ: op = "=="; break;
                case OP_NE: op = "!="; break;
            }
            printf("%s\n", op);
            break;
        }
        case AST_UNARY: {
            const char *op;
            switch (node->unary.op) {
                case OP_NEG: op = "-"; break;
                case OP_NOT: op = "~"; break;
            }
            printf("%s\n", op);
            break;
        }
    }
    
    // Left child
    if (node->type == AST_BINOP) {
        print_tree(node->binop.left, depth + 1);
    }
}

void print_connections(ASTNode *node, int depth) {
    if (!node) return;
    
    if (node->type == AST_BINOP) {
        // Right connection
        if (node->binop.right) {
            for (int i = 0; i < depth; i++) printf("    ");
            printf(" \\\n");
            print_connections(node->binop.right, depth + 1);
        }
        
        // Left connection
        if (node->binop.left) {
            for (int i = 0; i < depth; i++) printf("    ");
            printf(" /\n");
            print_connections(node->binop.left, depth + 1);
        }
    }
    else if (node->type == AST_UNARY) {
        // Unary connection
        if (node->unary.expr) {
            for (int i = 0; i < depth; i++) printf("    ");
            printf(" |\n");
            print_connections(node->unary.expr, depth + 1);
        }
    }
}

void print_ast(ASTNode *root) {
    printf("\nGenerated AST:\n");
    print_tree(root, 0);
    print_connections(root, 0);
}

int interpret_ast(ASTNode *node) {
    if (!node) return 0;

    switch (node->type) {
        case AST_INT:
            return node->int_value;

        case AST_IDENT: {
            // Look up variable value
            for (int i = 0; i < num_symbols; i++) {
                if (strcmp(symbol_table[i].name, node->name) == 0) {
                    return symbol_table[i].value;
                }
            }
            fprintf(stderr, "Undefined variable: %s\n", node->name);
            exit(1);
        }

        case AST_BINOP: {
            int left = interpret_ast(node->binop.left);
            int right = interpret_ast(node->binop.right);
            
            switch (node->binop.op) {
                case OP_ADD: return left + right;
                case OP_SUB: return left - right;
                case OP_MUL: return left * right;
                case OP_DIV: return left / right;
                case OP_LT: return left < right;
                case OP_GT: return left > right;
                case OP_LE: return left <= right;
                case OP_GE: return left >= right;
                case OP_EQ: return left == right;
                case OP_NE: return left != right;
            }
        }

        case AST_UNARY: {
            int expr = interpret_ast(node->unary.expr);
            switch (node->unary.op) {
                case OP_NEG: return -expr;
                case OP_NOT: return ~expr;
            }
        }

        case AST_ASSIGN: {
            // Evaluate the right-hand side
            int value = interpret_ast(node->assign.value);
            
            // Store in symbol table
            if (num_symbols >= MAX_SYMBOLS) {
                fprintf(stderr, "Symbol table full\n");
                exit(1);
            }
            
            // Check if variable exists
            for (int i = 0; i < num_symbols; i++) {
                if (strcmp(symbol_table[i].name, node->assign.name) == 0) {
                    symbol_table[i].value = value;
                    return value;
                }
            }
            
            // New variable
            symbol_table[num_symbols].name = node->assign.name;
            symbol_table[num_symbols].value = value;
            num_symbols++;
            return value;
        }

        default:
            fprintf(stderr, "Unknown node type: %d\n", node->type);
            exit(1);
    }
}

// void ast_codegen(ASTNode *node, FILE *out) {
//     if (!node) return;
//     switch (node->type) {
//         case AST_INT:
//             fprintf(out, "  mov rax, %d\n", node->int_value);
//             break;
//         case AST_IDENT:
//             fprintf(out, "  mov rax, [%s]\n", node->name);
//             break;
//         case AST_BINOP:
//             ast_codegen(node->binop.left, out);
//             fprintf(out, "  push rax\n");
//             ast_codegen(node->binop.right, out);
//             fprintf(out, "  pop rbx\n");
//             switch (node->binop.op) {
//                 case OP_ADD: fprintf(out, "  add rax, rbx\n"); break;
//                 case OP_SUB: fprintf(out, "  sub rax, rbx\n"); break;
//                 // ... other ops ...
//             }
//             break;
//         case AST_ASSIGN:
//             ast_codegen(node->assign.value, out);
//             fprintf(out, "  mov [%s], rax\n", node->assign.name);
//             break;
//     }
// }

void ast_free(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case AST_IDENT:
            free(node->name);  // Free the copied string
            break;
        case AST_BINOP:
            ast_free(node->binop.left);
            ast_free(node->binop.right);
            break;
         case AST_UNARY:
            ast_free(node->unary.expr);
            break;
        case AST_ASSIGN:
            ast_free(node->assign.value);
            free(node->assign.name);
            break;
        default:
            break;
    }
    free(node);
}