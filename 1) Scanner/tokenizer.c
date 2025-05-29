#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


// Helper: create a token with dynamically allocated text
Token make_token(TokenType type, const char* start, int length) {
    Token token;
    token.type = type;

    char* text = malloc(length + 1);
    memcpy(text, start, length);
    text[length] = '\0';

    token.text = text;  // writable copy
    token.intvalue = 0;
    return token;
}


// Helper: create a single-char token (text length = 1)
Token make_single_char_token(TokenType type, char c) {
    Token token;
    token.type = type;
    token.text = malloc(2);
    if (!token.text) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    token.text[0] = c;
    token.text[1] = '\0';
    token.intvalue = 0;
    return token;
}

Token get_next_token(FILE *source) {
    static char buffer[256];
    int c;

    // Skip whitespace except newlines
    do {
        c = fgetc(source);
        if (c == EOF) {
            return (Token){.type = T_EOF, .text = NULL, .intvalue = 0};
        }
    } while (c == ' ' || c == '\t');

    // Integer literal
    if(isdigit(c)){
        //I thought of replacing it with register input as it is much faster than in general
        int i = 0;
        buffer[i++] = (char)c;
        while ((c = fgetc(source)) != EOF && isdigit(c)) {
            if (i < 255) buffer[i++] = (char)c;
        }
        if (c != EOF) ungetc(c, source);
        buffer[i] = '\0';

        Token token = make_token(T_INT, buffer, i);
        token.intvalue = atoi(buffer);
        return token;
    }

    // Identifier (letters and underscores, then letters/digits/underscores)
    if (isalpha(c) || c == '_') {
        int i = 0;
        buffer[i++] = (char)c;
        while ((c = fgetc(source)) != EOF && (isalnum(c) || c == '_')) {
            if (i < 255) buffer[i++] = (char)c;
        }

        if (c == '(') return make_token(T_LPAREN, "(", 1);
        if (c == ')') return make_token(T_RPAREN, ")", 1);

        
        if (c != EOF) ungetc(c, source);
        buffer[i] = '\0';

        return make_token(T_IDENTIFIER, buffer, i);
    }

    // Single-character tokens
    switch (c) {
        case '+': return make_single_char_token(T_PLUS, '+');
        case '-': return make_single_char_token(T_MINUS, '-');
        case '*': return make_single_char_token(T_STAR, '*');
        case '/': return make_single_char_token(T_SLASH, '/');
        case '(': return make_single_char_token(T_LPAREN, '(');
        case ')': return make_single_char_token(T_RPAREN, ')');
        case '=': return make_single_char_token(T_ASSIGN, '=');
        case '\n': return make_single_char_token(T_NEWLINE, '\n');
        default:
            // Unknown token, just create a 1-char text token for error reporting
            return make_single_char_token(T_UNKNOWN, (char)c);
    }
}