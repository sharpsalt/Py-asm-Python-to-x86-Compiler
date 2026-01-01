#include"token.h"
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<stdbool.h>

#define LEXER_BUFFER_SIZE (1<<16) // 64KB buffer basically 2^10 stands for 1KB anf 2^6 stands for 64. so ye 64KB liye for fast Input(Early Memory Allocation)
#define MAX_TOKEN_LEN 256
#define MAX_INDENT_LEVEL 32

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
} LexerState;

static LexerState lexer;

const char *token_type_to_str(TokenType type)
{
    static const char *names[] = {
        "EOF", "INT", "FLOAT", "STRING", "IDENTIFIER",
        "+", "-", "*", "/", "**", "<<", ">>", "&", "|", "^", "~",
        "==", "!=", "<", ">", "<=", ">=",
        "=", "+=", "-=", "*=", "/=", "<<=", ">>=", "&=", "|=", "^=",
        "(", ")", "[", "]", ",", ":",
        "if", "else", "while", "for", "def", "return",
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
    lexer.indent_stack[lexer.indent_top] = 0; // Initial indent level
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
        }
    }
}

Token make_token(TokenType type, const char *start, int length)
{
    Token token;
    token.type = type;
    token.line = lexer.line;
    token.column = lexer.column - length;
    if (token.column < 1)
        token.column = 1;

    char *text = malloc(length + 1);
    if (!text)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    memcpy(text, start, length);
    text[length] = '\0';

    token.text = text;
    token.intvalue = 0;
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

static Token handle_indentation()
{
    int indent = 0;
    int c;

    while ((c = next_char()) == ' ' || c == '\t')
    {
        if (c == ' ')
        {
            indent += 1;
        }
        else if (c == '\t')
        {
            indent += 4; // Standard tab width
        }
    }

    backup_char();

    if (indent > lexer.indent_stack[lexer.indent_top])
    {
        if (lexer.indent_top >= MAX_INDENT_LEVEL - 1)
        {
            fprintf(stderr, "Error: Indentation level too deep at line %d\n", lexer.line);
            return make_token(T_UNKNOWN, "", 0);
        }
        lexer.indent_stack[++lexer.indent_top] = indent;
        return make_token(T_INDENT, "", 0);
    }
    else if (indent < lexer.indent_stack[lexer.indent_top])
    {
        lexer.indent_top--;
        return make_token(T_DEDENT, "", 0);
    }

    // No change in indentation
    return get_next_token();
}

Token get_next_token()
{
    int c;

    // Skip whitespace except newlines
    do
    {
        c = next_char();
        if (c == EOF)
        {
            // Generate DEDENT tokens for remaining indentation levels
            if (lexer.indent_top > 0)
            {
                lexer.indent_top--;
                return make_token(T_DEDENT, "", 0);
            }
            return (Token){.type = T_EOF, .text = NULL, .intvalue = 0};
        }
    } while (c == ' ' || c == '\t');

    // Handle newlines and indentation
    // Add this after newline handling
    if (c == ' ' || c == '\t')
    {
        return handle_indentation();
    }
    // In get_next_token()
    if (c == '\n')
    {
        // Count consecutive newlines but return just one NEWLINE token
        while ((c = next_char()) == '\n')
        {
            lexer.line++;
            lexer.column = 1;
        }
        backup_char();
        return make_single_char_token(T_NEWLINE, '\n');
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

        // Check for function calls
        // if (strcmp(token_buf, "if") == 0) return make_token(T_IF, token_buf, i);

        if (strcmp(token_buf, "int") == 0)
        {
            c = next_char();
            if (c == '(')
            {
                // Peek ahead for 'input()'
                Token next = get_next_token(); // Should return T_INPUT if "input()" is found
                if (next.type == T_INPUT)
                {
                    c = next_char();
                    if (c == ')')
                    {
                        free(next.text);
                        return make_token(T_INT_INPUT, "int(input())", 12);
                    }
                    backup_char();
                }
                free(next.text);
                backup_char(); // Restore '('
            }
            backup_char(); // Restore after 'int'
        }
        else if (strcmp(token_buf, "input") == 0)
        {
            c = next_char();
            if (c == '(')
            {
                c = next_char();
                if (c == ')')
                {
                    return make_token(T_INPUT, "input()", 7);
                }
                backup_char();
            }
            backup_char();
        }
        else if (strcmp(token_buf, "float") == 0)
        {
            c = next_char();
            if (c == '(')
            {
                Token next = get_next_token();
                if (next.type == T_INPUT)
                {
                    c = next_char();
                    if (c == ')')
                    {
                        free(next.text);
                        return make_token(T_FLOAT_INPUT, "float(input())", 14);
                    }
                    backup_char();
                }
                free(next.text);
                backup_char();
            }
            backup_char();
        }

        // Check for keywords
        if (strcmp(token_buf, "if") == 0)
            return make_token(T_IF, token_buf, i);
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
        if (strcmp(token_buf, "input") == 0)
            return make_token(T_INPUT, "input()", 7);

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
