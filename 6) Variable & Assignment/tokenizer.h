#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"
#include <stdio.h>     // For FILE
#include <stdbool.h>   // For bool
#include <stddef.h>    // For size_t

#define LEXER_BUFFER_SIZE (1<<16) 
#define MAX_TOKEN_LEN 256
#define MAX_INDENT_LEVEL 32

typedef struct
{
    FILE *source;
    char buffer[LEXER_BUFFER_SIZE];
    size_t pos;
    size_t len;
    int line;
    int column;
    int indent_stack[MAX_INDENT_LEVEL];
    int indent_top;
    bool pending_indent_check;
    int pending_dedents;
} LexerState;

Token get_next_token(void);
void init_lexer(FILE* source);

const char* token_type_to_str(TokenType type);
bool is_operator(TokenType type);
bool is_bitwise_op(TokenType type);
bool is_comparison(TokenType type);
bool is_keyword(TokenType type);
// Lexer state backup for peeking
LexerState save_lexer_state(void);
void restore_lexer_state(LexerState state);
#endif