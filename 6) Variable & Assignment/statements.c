#include "token.h"
#include "tokenizer.h"
#include <stdio.h>
#include <string.h>  // NEW: For strdup
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
    
    extern struct ASTNode *binexpr(int ptp);
    tree = binexpr(0);
    
    match(T_RPAREN);
    
    tree = mkastunary(A_PRINT, tree, 0, 0.0f, NULL, CurrentToken.line, CurrentToken.column);  // FIXED: Added NULL
    
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

        // FIXED: Added NULL
        return mkastnode(A_IF, cond, 
                         mkastnode(A_GLUE, body, further_else, 0, 0.0f, NULL, 
                                   keyword_line, keyword_column), 
                         0, 0.0f, NULL, keyword_line, keyword_column);
    }
}

// NEW: Parse assignment: IDENTIFIER ASSIGN expr
struct ASTNode *assignment_statement(void) {
    char *varname = NULL;
    struct ASTNode *expr = NULL;
    int assign_line = CurrentToken.line;
    int assign_column = CurrentToken.column;

    // Expect identifier
    if (CurrentToken.type != T_IDENTIFIER) {
        fprintf(stderr, "Expected identifier for assignment at line %d, column %d\n", 
                CurrentToken.line, CurrentToken.column);
        exit(1);
    }
    varname = strdup(CurrentToken.text);
    next_token();

    // Expect =
    match(T_ASSIGN);

    // Parse expression
    extern struct ASTNode *binexpr(int ptp);
    expr = binexpr(0);

    // Create A_ASSIGN node: left=var (as A_IDENT for name), right=expr
    struct ASTNode *var_node = mkastleaf(A_IDENT, 0, 0.0f, varname, assign_line, assign_column);
    return mkastnode(A_ASSIGN, var_node, expr, 0, 0.0f, NULL, assign_line, assign_column);
}

struct ASTNode *parse_block(void) {
    struct ASTNode *tree = NULL;
    
    while (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    if (CurrentToken.type != T_INDENT) {
        fprintf(stderr, "Expected indentation at line %d, column %d, got %s\n", 
                CurrentToken.line, CurrentToken.column, 
                token_type_to_str(CurrentToken.type));
        exit(1);
    }
    match(T_INDENT);
    
    while (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    tree = statement();
    
    while (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    while (CurrentToken.type != T_DEDENT && CurrentToken.type != T_EOF) {
        struct ASTNode *next_stmt = statement();
        tree = mkastnode(A_GLUE, tree, next_stmt, 0, 0.0f, NULL, CurrentToken.line, CurrentToken.column);  // FIXED: Added NULL
        
        while (CurrentToken.type == T_NEWLINE) {
            next_token();
        }
    }
    
    if (CurrentToken.type == T_DEDENT) {
        match(T_DEDENT);
    }
    
    return tree;
}

struct ASTNode *if_statement(void) {
    int if_line = CurrentToken.line;
    int if_column = CurrentToken.column;
    
    match(T_IF);
    
    extern struct ASTNode *binexpr(int ptp);
    struct ASTNode *condition = binexpr(0);
    match(T_COLON);
    
    if (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    struct ASTNode *if_body = parse_block();
    
    struct ASTNode *else_part = NULL;
    if (CurrentToken.type == T_ELIF || CurrentToken.type == T_ELSE) {
        else_part = parse_elif_or_else();
    }
    
    // FIXED: Added NULL
    return mkastnode(A_IF, condition, 
                     mkastnode(A_GLUE, if_body, else_part, 0, 0.0f, NULL, 
                               if_line, if_column), 
                     0, 0.0f, NULL, if_line, if_column);
}

struct ASTNode *statement(void) {
    switch (CurrentToken.type) {
        case T_PRINT:
            return print_statement();
        case T_IF:
            return if_statement();
        case T_IDENTIFIER:  // NEW: Check if followed by ASSIGN
            {
                Token old_token = CurrentToken;
                LexerState old_lexer = save_lexer_state();  // NEW: Save lexer state
                next_token();  // Peek ahead
                if (CurrentToken.type == T_ASSIGN) {
                    // Restore state and token, then parse assignment
                    restore_lexer_state(old_lexer);
                    CurrentToken = old_token;
                    return assignment_statement();
                } else {
                    // Restore and error (standalone IDs not supported yet)
                    restore_lexer_state(old_lexer);
                    CurrentToken = old_token;
                    fprintf(stderr, "Syntax error: unexpected identifier '%s' at line %d, column %d\n", 
                            CurrentToken.text, CurrentToken.line, CurrentToken.column);
                    exit(1);
                }
            }
        default:  // Elif handled in chain, not here
            fprintf(stderr, "Syntax error: unexpected token %s at line %d, column %d\n", 
                    token_type_to_str(CurrentToken.type), CurrentToken.line, CurrentToken.column);
            exit(1);
    }
}

struct ASTNode *statements(void) {
    struct ASTNode *tree = statement();
    
    while (CurrentToken.type == T_NEWLINE) {
        next_token();
    }
    
    if (CurrentToken.type != T_EOF && CurrentToken.type != T_DEDENT) {
        struct ASTNode *more = statements();
        tree = mkastnode(A_GLUE, tree, more, 0, 0.0f, NULL, CurrentToken.line, CurrentToken.column);  // FIXED: Added NULL
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