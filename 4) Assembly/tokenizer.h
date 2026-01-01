#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"

Token get_next_token(void);
void init_lexer(FILE* source);

const char* token_type_to_str(TokenType type);
bool is_operator(TokenType type);
bool is_bitwise_op(TokenType type);
bool is_comparison(TokenType type);
bool is_keyword(TokenType type);

#endif