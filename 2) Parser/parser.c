#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "ast.h"
#include "../Scanner/tokenizer.h"

static Parser parser;

void init_parser(FILE* source) {
    init_lexer(source);
    parser.current = get_next_token();
    parser.previous = (Token){0};
    parser.had_error = false;
    parser.panic_mode = false;
}

static void advance() {
    parser.previous = parser.current;
    
    if (parser.current.text) free(parser.current.text);
    
    parser.current = get_next_token();
    if (parser.current.type == T_UNKNOWN) {
        fprintf(stderr, "Unexpected token at line %d\n", parser.current.line);
        parser.had_error = true;
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }
    
    fprintf(stderr, "Error at line %d: %s\n", parser.current.line, message);
    parser.had_error = true;
}

ASTNode* parse_expression();
ASTNode* parse_assignment();
ASTNode* parse_equality();
ASTNode* parse_comparison();
ASTNode* parse_term();
ASTNode* parse_factor();
ASTNode* parse_unary();
ASTNode* parse_primary();

ASTNode* parse_program() {
    init_parser(parser.current.text ? stdin : NULL); // Handle repl case
    
    ASTNode* node = NULL;
    while (parser.current.type != T_EOF) {
        ASTNode* stmt = parse_statement();
        if (!node) {
            node = stmt;
        } else {
            // For multiple statements, create a block node
            node = new_block_node(node, stmt);
        }
        consume(T_NEWLINE, "Expect newline after statement");
    }
    
    return node;
}

ASTNode* parse_statement() {
    if (parser.current.type == T_IF) return parse_if();
    if (parser.current.type == T_WHILE) return parse_while();
    if (parser.current.type == T_DEF) return parse_function();
    if (parser.current.type == T_RETURN) return parse_return();
    if (parser.current.type == T_PRINT) return parse_print();
    return parse_assignment();
}

ASTNode* parse_assignment() {
    ASTNode* expr = parse_equality();
    
    if (parser.current.type == T_ASSIGN) {
        if (expr->type != AST_IDENT) {
            fprintf(stderr, "Invalid assignment target\n");
            parser.had_error = true;
            return expr;
        }
        
        char* name = strdup(expr->name);
        ast_free(expr); // Free the ident node
        
        advance(); // Consume =
        ASTNode* value = parse_assignment();
        return new_assign_node(name, value);
    }
    
    return expr;
}

ASTNode* parse_expression() {
    return parse_assignment();
}

ASTNode* parse_equality() {
    ASTNode* expr = parse_comparison();
    
    while (parser.current.type == T_EQ || parser.current.type == T_NE) {
        BinOpType op = parser.current.type == T_EQ ? OP_EQ : OP_NE;
        advance();
        ASTNode* right = parse_comparison();
        expr = new_binop_node(op, expr, right);
    }
    
    return expr;
}

ASTNode* parse_comparison() {
    ASTNode* expr = parse_term();
    
    while (parser.current.type >= T_LT && parser.current.type <= T_GE) {
        BinOpType op;
        switch (parser.current.type) {
            case T_LT: op = OP_LT; break;
            case T_GT: op = OP_GT; break;
            case T_LE: op = OP_LE; break;
            case T_GE: op = OP_GE; break;
            default: break; // Shouldn't happen
        }
        advance();
        ASTNode* right = parse_term();
        expr = new_binop_node(op, expr, right);
    }
    
    return expr;
}

ASTNode* parse_term() {
    ASTNode* expr = parse_factor();
    
    while (parser.current.type == T_PLUS || parser.current.type == T_MINUS) {
        BinOpType op = parser.current.type == T_PLUS ? OP_ADD : OP_SUB;
        advance();
        ASTNode* right = parse_factor();
        expr = new_binop_node(op, expr, right);
    }
    
    return expr;
}

ASTNode* parse_factor() {
    ASTNode* expr = parse_unary();
    
    while (parser.current.type == T_STAR || parser.current.type == T_SLASH) {
        BinOpType op = parser.current.type == T_STAR ? OP_MUL : OP_DIV;
        advance();
        ASTNode* right = parse_unary();
        expr = new_binop_node(op, expr, right);
    }
    
    return expr;
}

ASTNode* parse_unary() {
    if (parser.current.type == T_MINUS || parser.current.type == T_BIT_NOT) {
        UnaryOpType op = parser.current.type == T_MINUS ? OP_NEG : OP_NOT;
        advance();
        ASTNode* right = parse_unary();
        return new_unary_node(op, right);
    }
    
    return parse_primary();
}

ASTNode* parse_primary() {
    switch (parser.current.type) {
        case T_INT:
            return new_int_node(parser.current.intvalue);
        case T_FLOAT:
            return new_float_node(parser.current.floatvalue);
        case T_STRING:
            return new_string_node(strdup(parser.current.text));
        case T_IDENTIFIER:
            return new_ident_node(strdup(parser.current.text));
        case T_LPAREN: {
            advance();
            ASTNode* expr = parse_expression();
            consume(T_RPAREN, "Expect ')' after expression");
            return expr;
        }
        case T_INT_INPUT:
            return new_input_node(T_INT_INPUT);
        case T_FLOAT_INPUT:
            return new_input_node(T_FLOAT_INPUT);
        default:
            fprintf(stderr, "Unexpected token in expression\n");
            parser.had_error = true;
            return NULL;
    }
}

// Error recovery helper
void parser_sync() {
    parser.panic_mode = false;
    
    while (parser.current.type != T_EOF) {
        if (parser.previous.type == T_NEWLINE) return;
        
        switch (parser.current.type) {
            case T_DEF:
            case T_IF:
            case T_WHILE:
            case T_PRINT:
            case T_RETURN:
                return;
            default:
                advance();
        }
    }
}