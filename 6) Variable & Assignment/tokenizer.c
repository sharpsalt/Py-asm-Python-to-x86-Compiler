#include"token.h"
#include"tokenizer.h"
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<stdbool.h>


bool is_operator(TokenType type)
{
    return (type>=T_PLUS && type<=T_RSHIFT_EQ) || type==T_BIT_NOT;
}

bool is_bitwise_op(TokenType type)
{
    return (type>=T_LSHIFT && type<=T_BIT_NOT);
}

bool is_comparison(TokenType type)
{
    return (type>=T_EQ && type<=T_GE);
}

bool is_keyword(TokenType type)
{
    return (type>=T_IF && type<=T_PRINT);
}

static LexerState lexer;

const char *token_type_to_str(TokenType type)
{
    static const char *names[] = {
        "EOF", "INT", "FLOAT", "STRING", "IDENTIFIER",
        "+", "-", "*", "/", "**", "<<", ">>", "&", "|", "^", "~",
        "==", "!=", "<", ">", "<=", ">=",
        "=", "+=", "-=", "*=", "/=", "<<=", ">>=", "&=", "|=", "^=",
        "(", ")", "[", "]", ",", ":",
        "if", "else", "while", "for", "def", "return", "elif",
        "input", "int(input)", "float(input)", "print",
        "NEWLINE", "INDENT", "DEDENT", "UNKNOWN"};
    return names[type];
}

void init_lexer(FILE *source)
{
    lexer.source = source;
    lexer.pos = 0;
    lexer.len = fread(lexer.buffer, 1, LEXER_BUFFER_SIZE, source);
    lexer.line = 1;
    lexer.column = 1;
    lexer.indent_top = 0;
    lexer.indent_stack[lexer.indent_top] = 0;
    lexer.pending_indent_check = false;
    lexer.pending_dedents = 0;
}

static inline int next_char()
{
    if (lexer.pos >= lexer.len)
    {
        if (lexer.len < LEXER_BUFFER_SIZE)
        {
            return EOF;
        }
        lexer.len = fread(lexer.buffer, 1, LEXER_BUFFER_SIZE, lexer.source);
        lexer.pos = 0;
        if (lexer.len == 0)
            return EOF;
    }

    int c = lexer.buffer[lexer.pos++];
    if (c == '\n')
    {
        lexer.line++;
        lexer.column = 1;
    }
    else
    {
        lexer.column++;
    }
    return c;
}

static inline void backup_char()
{
    if (lexer.pos > 0)
    {
        lexer.pos--;
        lexer.column--;
        if (lexer.buffer[lexer.pos] == '\n')
        {
            lexer.line--;
            lexer.column = lexer.column ? lexer.column - 1 : 1;  // Approx
        }
    }
}

Token make_token(TokenType type, const char *start, int length) {
    Token token;
    token.type = type;
    token.line = lexer.line;
    token.column = lexer.column - length;
    if (token.column < 1)
        token.column = 1;

    if (length > 0 && start != NULL) {
        char *text = malloc(length + 1);
        if (!text) {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
        memcpy(text, start, length);
        text[length] = '\0';
        token.text = text;
    } else {
        token.text = malloc(1);
        token.text[0] = '\0';
    }
    
    token.intvalue = 0;
    token.floatvalue = 0.0f;
    return token;
}

Token make_single_char_token(TokenType type, char c)
{
    Token token;
    token.type = type;
    token.line = lexer.line;
    token.column = lexer.column - 1;

    token.text = malloc(2);
    if (!token.text)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    token.text[0] = c;
    token.text[1] = '\0';
    token.intvalue = 0;
    token.floatvalue = 0.0f;
    return token;
}

static Token handle_string_literal(char quote)
{
    char token_buf[MAX_TOKEN_LEN];
    int i = 0;
    int c;

    while ((c = next_char()) != quote && c != EOF && i < MAX_TOKEN_LEN - 1)
    {
        if (c == '\\')
        {
            c = next_char();
            switch (c)
            {
            case 'n':
                token_buf[i++] = '\n';
                break;
            case 't':
                token_buf[i++] = '\t';
                break;
            case 'r':
                token_buf[i++] = '\r';
                break;
            case '\\':
                token_buf[i++] = '\\';
                break;
            case '\'':
                token_buf[i++] = '\'';
                break;
            case '"':
                token_buf[i++] = '"';
                break;
            default:
                token_buf[i++] = '\\';
                token_buf[i++] = c;
                break;
            }
        }
        else
        {
            token_buf[i++] = (char)c;
        }
    }

    if (c != quote)
    {
        fprintf(stderr, "Error: Unterminated string literal at line %d\n", lexer.line);
        return make_token(T_UNKNOWN, token_buf, i);
    }

    return make_token(T_STRING, token_buf, i);
}

static Token handle_number(int first_char)
{
    char token_buf[MAX_TOKEN_LEN];
    int i = 0;
    int c = first_char;
    bool is_float = false;

    token_buf[i++] = (char)c;

    while ((c = next_char()) != EOF && i < MAX_TOKEN_LEN - 1)
    {
        if (isdigit(c))
        {
            token_buf[i++] = (char)c;
        }
        else if (c == '.')
        {
            if (is_float)
                break;
            is_float = true;
            token_buf[i++] = (char)c;
        }
        else
        {
            backup_char();
            break;
        }
    }

    token_buf[i] = '\0';

    if (is_float)
    {
        double value = atof(token_buf);
        Token token = make_token(T_FLOAT, token_buf, i);
        token.floatvalue = value;
        return token;
    }
    else
    {
        int value = atoi(token_buf);
        Token token = make_token(T_INT, token_buf, i);
        token.intvalue = value;
        return token;
    }
}

LexerState save_lexer_state(void) {
    return lexer;
}

void restore_lexer_state(LexerState state) {
    lexer = state;
}

Token get_next_token()
{
    int c;

    // Handle pending dedents first
    if (lexer.pending_dedents > 0) {
        lexer.pending_dedents--;
        Token t = make_token(T_DEDENT, "DEDENT", 6);
        t.line = lexer.line;
        t.column = 1;
        return t;
    }

    // Handle pending indent check (after NEWLINE)
    if (lexer.pending_indent_check) {
        lexer.pending_indent_check = false;
        int indent = 0;
        int ch;
        bool hit_eof = false;

        while (true) {
            ch = next_char();
            if (ch == EOF) {
                hit_eof = true;
                break;
            }
            if (ch == ' ') {
                indent += 1;
            } else if (ch == '\t') {
                indent += 4;
            } else {
                backup_char();
                break;
            }
        }

        if (hit_eof) {
            int num = lexer.indent_top;
            lexer.indent_top = 0;
            if (num > 0) {
                lexer.pending_dedents = num;
                lexer.pending_dedents--;  // Consume one now
                Token t = make_token(T_DEDENT, "DEDENT", 6);
                t.line = lexer.line;
                t.column = 1;
                return t;
            }
            return make_token(T_EOF, "", 0);
        }

        int current = (lexer.indent_top >= 0) ? lexer.indent_stack[lexer.indent_top] : 0;
        if (indent > current) {
            if (lexer.indent_top >= MAX_INDENT_LEVEL - 1) {
                fprintf(stderr, "Indentation level too deep\n");
                exit(1);
            }
            lexer.indent_stack[++lexer.indent_top] = indent;
            Token t = make_token(T_INDENT, "INDENT", 6);
            t.line = lexer.line;
            t.column = 1;
            return t;
        } else if (indent < current) {
            int num = 0;
            while (lexer.indent_top > 0 && lexer.indent_stack[lexer.indent_top] > indent) {
                lexer.indent_top--;
                num++;
            }
            lexer.pending_dedents = num;
            lexer.pending_dedents--;  // Consume one now
            Token t = make_token(T_DEDENT, "DEDENT", 6);
            t.line = lexer.line;
            t.column = 1;
            return t;
        }
        // Same level: proceed to tokenize (pos backed to first non-white)
    }

    // Get next char, skipping \r
    do {
        c = next_char();
        if (c == EOF) {
            // Handle remaining dedents at EOF
            int num = lexer.indent_top;
            lexer.indent_top = 0;
            if (num > 0) {
                lexer.pending_dedents = num;
                lexer.pending_dedents--;  // One now
                Token t = make_token(T_DEDENT, "DEDENT", 6);
                t.line = lexer.line;
                t.column = 1;
                return t;
            }
            return make_token(T_EOF, "", 0);
        }
    } while (c == '\r');

    // Handle newlines
    if (c == '\n') {
        lexer.pending_indent_check = true;
        return make_single_char_token(T_NEWLINE, '\n');
    }

    // Skip other whitespace
    while (c == ' ' || c == '\t') {
        c = next_char();
        if (c == EOF) {
            int num = lexer.indent_top;
            lexer.indent_top = 0;
            if (num > 0) {
                lexer.pending_dedents = num;
                lexer.pending_dedents--;
                Token t = make_token(T_DEDENT, "DEDENT", 6);
                t.line = lexer.line;
                t.column = 1;
                return t;
            }
            return make_token(T_EOF, "", 0);
        }
    }

    // Integer or float literal
    if (isdigit(c))
    {
        return handle_number(c);
    }

    // String literals
    if (c == '"' || c == '\'')
    {
        return handle_string_literal(c);
    }

    // Identifiers and keywords
    if (isalpha(c) || c == '_')
    {
        char token_buf[MAX_TOKEN_LEN];
        int i = 0;
        do
        {
            token_buf[i++] = (char)c;
            c = next_char();
        } while ((isalnum(c) || c == '_') && i < MAX_TOKEN_LEN - 1);

        backup_char();
        token_buf[i] = '\0';

        // Check for keywords
        if (strcmp(token_buf, "if") == 0)
            return make_token(T_IF, token_buf, i);
        if (strcmp(token_buf, "elif") == 0)
            return make_token(T_ELIF, token_buf, i);
        if (strcmp(token_buf, "else") == 0)
            return make_token(T_ELSE, token_buf, i);
        if (strcmp(token_buf, "while") == 0)
            return make_token(T_WHILE, token_buf, i);
        if (strcmp(token_buf, "for") == 0)
            return make_token(T_FOR, token_buf, i);
        if (strcmp(token_buf, "def") == 0)
            return make_token(T_DEF, token_buf, i);
        if (strcmp(token_buf, "return") == 0)
            return make_token(T_RETURN, token_buf, i);
        if (strcmp(token_buf, "print") == 0)
            return make_token(T_PRINT, token_buf, i);
            
        return make_token(T_IDENTIFIER, token_buf, i);
    }

    // Operators and punctuation
    switch (c)
    {
    case '+':
        c = next_char();
        if (c == '=')
            return make_token(T_PLUS_EQ, "+=", 2);
        backup_char();
        return make_single_char_token(T_PLUS, '+');

    case '-':
        c = next_char();
        if (c == '=')
            return make_token(T_MINUS_EQ, "-=", 2);
        backup_char();
        return make_single_char_token(T_MINUS, '-');

    case '*':
        c = next_char();
        if (c == '*')
            return make_token(T_POWER, "**", 2);
        if (c == '=')
            return make_token(T_STAR_EQ, "*=", 2);
        backup_char();
        return make_single_char_token(T_STAR, '*');

    case '/':
        c = next_char();
        if (c == '=')
            return make_token(T_SLASH_EQ, "/=", 2);
        backup_char();
        return make_single_char_token(T_SLASH, '/');

    case '<':
        c = next_char();
        if (c == '<')
        {
            c = next_char();
            if (c == '=')
                return make_token(T_LSHIFT_EQ, "<<=", 3);
            backup_char();
            return make_token(T_LSHIFT, "<<", 2);
        }
        if (c == '=')
            return make_token(T_LE, "<=", 2);
        backup_char();
        return make_single_char_token(T_LT, '<');

    case '>':
        c = next_char();
        if (c == '>')
        {
            c = next_char();
            if (c == '=')
                return make_token(T_RSHIFT_EQ, ">>=", 3);
            backup_char();
            return make_token(T_RSHIFT, ">>", 2);
        }
        if (c == '=')
            return make_token(T_GE, ">=", 2);
        backup_char();
        return make_single_char_token(T_GT, '>');

    case '=':
        c = next_char();
        if (c == '=')
            return make_token(T_EQ, "==", 2);
        backup_char();
        return make_single_char_token(T_ASSIGN, '=');

    case '!':
        c = next_char();
        if (c == '=')
            return make_token(T_NE, "!=", 2);
        backup_char();
        return make_single_char_token(T_UNKNOWN, '!');

    case '&':
        c = next_char();
        if (c == '=')
            return make_token(T_BIT_AND_EQ, "&=", 2);
        backup_char();
        return make_single_char_token(T_BIT_AND, '&');

    case '|':
        c = next_char();
        if (c == '=')
            return make_token(T_BIT_OR_EQ, "|=", 2);
        backup_char();
        return make_single_char_token(T_BIT_OR, '|');

    case '^':
        c = next_char();
        if (c == '=')
            return make_token(T_BIT_XOR_EQ, "^=", 2);
        backup_char();
        return make_single_char_token(T_BIT_XOR, '^');

    case '~':
        return make_single_char_token(T_BIT_NOT, '~');

    case '(':
        return make_single_char_token(T_LPAREN, '(');
    case ')':
        return make_single_char_token(T_RPAREN, ')');
    case '[':
        return make_single_char_token(T_LBRACKET, '[');
    case ']':
        return make_single_char_token(T_RBRACKET, ']');
    case ',':
        return make_single_char_token(T_COMMA, ',');
    case ':':
        return make_single_char_token(T_COLON, ':');

    default:
        return make_single_char_token(T_UNKNOWN, (char)c);
    }
}