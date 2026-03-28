# 1) Scanner (Lexical Analysis)

Welcome to the first stage of our zero-dependency Python-to-x86 compiler written in C. This directory contains the **Scanner** (also known as a Lexer or Tokenizer).

## The Goal of this Stage
A compiler cannot understand raw text (like `x = 5 + 3`) directly. The first step in compiling source code is breaking it down into a sequence of meaningful chunks called **Tokens**. The scanner reads the source file character by character and groups them into these tokens.

## Key Concepts Explained

### 1. Tokens (`token.h`)
Tokens are the fundamental vocabulary of our compiler. Instead of dealing with strings, the compiler deals with enumerated types. For example:
- The character `+` becomes `T_PLUS`.
- The string `123` becomes `T_INT` with an associated integer value of `123`.
- Words like `print` or `if` become specific keyword tokens.

### 2. The Lexer Loop (`tokenizer.c`)
The workhorse of this directory is `tokenizer.c`. Its main function is `get_next_token()`. Every time the parser needs a new piece of information, it calls this function.
- **Skipping Whitespace**: It skips over spaces, tabs, and comments.
- **Scanning Numbers**: When it encounters a digit, it enters a loop to read consecutive digits, converting the ASCII string into an actual C integer.
- **Scanning Identifiers**: When it encounters a letter, it reads the whole word to see if it's a keyword or a variable name.
- **Tracking Position**: It keeps track of the current `line` and `column`. This is crucial for throwing precise syntax errors later.

## How to Test and Run
The `main.c` file in this directory typically initializes the scanner and calls `get_next_token()` in a loop, printing out the tokens it finds. This allows us to verify that the lexer correctly slices up the input program.
