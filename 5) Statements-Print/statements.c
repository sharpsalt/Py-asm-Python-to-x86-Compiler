#include "token.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "ast.h"
#include "statements.h" 

// Forward declarations
static void next_token(void);
static void match(TokenType expected);
struct ASTNode *parse_elif_or_else(void);  // New helper

static void next_token(void) {
    CurrentToken = get_next_token();
}

static void match(TokenType expected) {
    if (CurrentToken.type == expected) {
        next_token();
    } else {
        fprintf(stderr, "Expected %s, got %s at line %d, column %d\n", 
                token_type_to_str(expected), token_type_to_str(CurrentToken.type),
                CurrentToken.line, CurrentToken.column);
        exit(1);
    }
}

// Parse a print statement: print(expression)
struct ASTNode *print_statement(void) {
    struct ASTNode *tree;
    
    match(T_PRINT);
    match(T_LPAREN);
    
    // Parse the expression to print - use external binexpr function
    extern struct ASTNode *binexpr(int ptp);  // Declare the external function
    tree = binexpr(0);
    
    match(T_RPAREN);
    
    // Create a print AST node with the expression as its child
    tree = mkastunary(A_PRINT, tree, 0, 0.0f, CurrentToken.line, CurrentToken.column);
    
    return tree;
}

// New helper: Parse elif or else chain (recursive for chaining)
struct ASTNode *parse_elif_or_else(void) {
    struct ASTNode *result;
    int keyword_line = CurrentToken.line;
    int keyword_column = CurrentToken.column;

    if (CurrentToken.type == T_ELSE) {
        match(T_ELSE);
        match(T_COLON);
        if (CurrentToken.type == T_NEWLINE) {
            next_token();
        }
        // Just return the else body (no unnecessary GLUE)
        return parse_block();
    } else {  // T_ELIF
        match(T_ELIF);
        extern struct ASTNode *binexpr(int ptp);
        struct ASTNode *cond = binexpr(0);
        match(T_COLON);
        if (CurrentToken.type == T_NEWLINE) {
            next_token();
        }
        struct ASTNode *body = parse_block();

        struct ASTNode *further_else = NULL;
        if (CurrentToken.type == T_ELIF || CurrentToken.type == T_ELSE) {
            further_else = parse_elif_or_else();
        }

        // Build nested IF for this elif: if cond (body, further_else)
        return mkastnode(A_IF, cond, 
                         mkastnode(A_GLUE, body, further_else, 0, 0.0f, 
                                   keyword_line, keyword_column), 
                         0, 0.0f, keyword_line, keyword_column);
    }
}

struct ASTNode *parse_block(void) {
    struct ASTNode *tree = NULL;
    
    // After colon, we expect NEWLINE then INDENT
    // Skip any newlines first
    while (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    // Now expect INDENT
    if (CurrentToken.type != T_INDENT) {
        fprintf(stderr, "Expected indentation at line %d, column %d, got %s\n", 
                CurrentToken.line, CurrentToken.column, 
                token_type_to_str(CurrentToken.type));
        exit(1);
    }
    match(T_INDENT);
    
    // Skip any newlines after indent
    while (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    // Parse first statement
    tree = statement();
    
    // Skip newlines between statements
    while (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    // Handle multiple statements in block
    while (CurrentToken.type != T_DEDENT && CurrentToken.type != T_EOF) {
        struct ASTNode *next_stmt = statement();
        tree = mkastnode(A_GLUE, tree, next_stmt, 0, 0.0f, CurrentToken.line, CurrentToken.column);
        
        // Skip newlines between statements
        while (CurrentToken.type == T_NEWLINE) {
            next_token();
        }
    }
    
    // Expect DEDENT
    if (CurrentToken.type == T_DEDENT) {
        match(T_DEDENT);
    }
    
    return tree;
}

struct ASTNode *if_statement(void) {
    int if_line = CurrentToken.line;
    int if_column = CurrentToken.column;
    
    match(T_IF);
    
    // Parse condition - use external binexpr function
    extern struct ASTNode *binexpr(int ptp);  // Declare the external function
    struct ASTNode *condition = binexpr(0);
    match(T_COLON);
    
    // Skip newline after colon
    if (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    // Parse if body
    struct ASTNode *if_body = parse_block();
    
    // Handle else/elif chain
    struct ASTNode *else_part = NULL;
    if (CurrentToken.type == T_ELIF || CurrentToken.type == T_ELSE) {
        else_part = parse_elif_or_else();
    }
    
    return mkastnode(A_IF, condition, 
                     mkastnode(A_GLUE, if_body, else_part, 0, 0.0f, 
                               if_line, if_column), 
                     0, 0.0f, if_line, if_column);
}

struct ASTNode *statement(void) {
    switch (CurrentToken.type) {
        case T_PRINT:
            return print_statement();
        case T_IF:
            return if_statement();
        default:  // Elif handled in chain, not here
            fprintf(stderr, "Syntax error: unexpected token %s at line %d, column %d\n", 
                    token_type_to_str(CurrentToken.type), CurrentToken.line, CurrentToken.column);
            exit(1);
    }
}

struct ASTNode *statements(void) {
    struct ASTNode *tree = statement();
    
    // Skip newlines
    while (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    // If more statements, glue them
    if (CurrentToken.type != T_EOF && CurrentToken.type != T_DEDENT) {
        struct ASTNode *more = statements();
        tree = mkastnode(A_GLUE, tree, more, 0, 0.0f, CurrentToken.line, CurrentToken.column);
    }
    
    return tree;
}

// Main parsing function for statements
struct ASTNode *parse_statements(FILE *input) {
    init_lexer(input);
    next_token();
    
    // Skip any initial newlines
    while (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    // If file is empty or only newlines
    if (CurrentToken.type == T_EOF) {
        fprintf(stderr, "Empty input file\n");
        exit(1);
    }
    
    return statements();
}