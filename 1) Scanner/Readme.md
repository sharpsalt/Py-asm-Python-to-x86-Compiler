
# Py-asm: Python to x86 Tokenizer (Scanner)

This project is a component of a Python-to-x86 compiler. It implements a **tokenizer (lexer)** in C, which reads Python source code and breaks it into tokens for further compilation or analysis.

## Features
- Tokenizes Python code into integers, floats, strings, identifiers, operators, keywords, and more
- Handles indentation and dedentation (Python significant whitespace)
- Supports basic Python syntax: arithmetic, assignment, comparison, bitwise, and control flow tokens
- Prints token type, value, line, and column for each token

## File Structure
- `main.c` — Entry point, runs the tokenizer on `input.py`
- `tokenizer.c` / `tokenizer.h` — Implementation and interface for the lexer
- `token.h` — Token and token type definitions
- `input.py` — Sample Python input file
- `tokenizer` — Compiled binary (after build)

## Usage
1. **Prepare your Python input:**
   - Edit `input.py` with the Python code you want to tokenize.
2. **Build the tokenizer:**
   ```bash
   gcc -o tokenizer main.c tokenizer.c -Wall
   ```
3. **Run the tokenizer:**
   ```bash
   ./tokenizer
   ```
   The output will list all tokens found in `input.py` with their type, value, line, and column.

## Example
Given `input.py`:
```python
y = (3 + 5) * 10 - 20 / 4
```
The output will be a list of tokens like:
```
Token: IDENTIFIER   Line: 1   Col: 1   Text: 'y'
Token: ASSIGN       Line: 1   Col: 3   Text: '='
Token: LPAREN       Line: 1   Col: 5   Text: '('
Token: INT          Line: 1   Col: 6   Text: '3' Value: 3
...etc.
```

## Notes
- Only `input.py` is tokenized; change its contents to test different code.
- This is a **scanner/tokenizer only**; parsing and code generation are not included here.
- For educational and experimental use.

## License
MIT License (or specify your own)

---
*Part of a Python-to-x86 compiler project.*

