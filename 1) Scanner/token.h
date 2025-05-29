#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>   // For FILE*

typedef enum{
    T_EOF,        //For End Of File
    T_INT,        //For intvalue
    T_IDENTIFIER, //For identifier like x,y etc
    T_PLUS,       //For +
    T_MINUS,      //For -
    T_STAR,       //For *
    T_SLASH,      //For /
    T_LPAREN,     //For (
    T_RPAREN,     //For )
    T_ASSIGN,     //For =
    T_NEWLINE,    //For Newline (Basically \n)
    T_UNKNOWN 
}TokenType;

typedef struct{
    TokenType type;
    char* text;
    int intvalue;
} Token;

Token get_next_token(FILE *source);

#endif // TOKEN_H
