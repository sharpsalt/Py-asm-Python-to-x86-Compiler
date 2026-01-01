#include<stdio.h>
#include<stdbool.h>

#ifndef TOKEN_H
#define TOKEN_H
typedef enum {
    T_EOF,        // End Of File
    T_INT,        // Integer literal
    T_FLOAT,      // Floating Point literal
    T_STRING,     // String literal
    T_IDENTIFIER, // Variable like x,y etc
    T_PLUS,       // +
    T_MINUS,      // -
    T_STAR,       // *
    T_SLASH,      // /
    T_POWER,      // **
    T_LSHIFT,     // <<
    T_RSHIFT,     // >>
    T_BIT_AND,    // &
    T_BIT_OR,     // |
    T_BIT_XOR,    // ^
    T_BIT_NOT,    // ~
    T_EQ,         // ==
    T_NE,         // !=
    T_LT,         // <
    T_GT,         // >
    T_LE,         // <=
    T_GE,         // >=
    T_ASSIGN,     // =
    T_PLUS_EQ,    // +=
    T_MINUS_EQ,   // -=
    T_STAR_EQ,    // *=
    T_SLASH_EQ,   // /=
    T_LSHIFT_EQ,  // <<=
    T_RSHIFT_EQ,  // >>=
    T_BIT_AND_EQ, // &=
    T_BIT_OR_EQ,  // |=
    T_BIT_XOR_EQ, // ^=
    T_LPAREN,     // (
    T_RPAREN,     // )
    T_LBRACKET,   // [
    T_RBRACKET,   // ]
    T_COMMA,      // ,
    T_COLON,      // :
    T_IF,         // if
    T_ELSE,       // else
    T_WHILE,      // while
    T_FOR,        // for
    T_DEF,        // def
    T_RETURN,     // return
    T_INPUT,      // input()
    T_INT_INPUT,  // int(input())
    T_FLOAT_INPUT,// float(input())
    T_PRINT,      // print
    T_NEWLINE,    // Newline (\n)
    T_INDENT,     // Indentation
    T_DEDENT,     // Dedentation
    T_UNKNOWN     // Unknown token
} TokenType;

typedef struct {
    TokenType type;
    char* text;
    union {
        int intvalue;      //For T_INT
        double floatvalue; //For T_FLOAT
    };
    int line;  //For Soruce line number
    int column; //For Source column number
} Token;

void init_lexer(FILE* source);
Token get_next_token();
const char* token_type_to_str(TokenType type);

bool is_operator(TokenType type);
bool is_bitwise_op(TokenType type);
bool is_comparison(TokenType type);
bool is_keyword(TokenType type);

#endif // TOKEN_H