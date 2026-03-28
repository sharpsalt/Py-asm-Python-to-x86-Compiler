# 3) Precedence Parser (Pratt Parsing)

This directory solves a major problem from the previous stage: **Operator Precedence**. In mathematics, `2 + 3 * 4` must evaluate to `14`, not `20`. The multiplication must happen first.

## The Goal of this Stage
We need our parser to build an AST that inherently respects the order of operations. For `2 + 3 * 4`, the `+` must be the root node, and the `*` must be its right child, ensuring `3 * 4` is evaluated before being added to `2`.

## Key Concepts Explained

### 1. Pratt Parsing (Top-Down Operator Precedence)
The easiest and most elegant way to solve this in a handwritten recursive descent parser is using a technique called Pratt Parsing (invented by Vaughan Pratt).
The core idea is assigning a "binding power" or **precedence level** to every operator.

### 2. Implementation in `expression.c`
- **`op_precedence(TokenType)`**: A new function is introduced that returns an integer. `*` and `/` might return `20`, while `+` and `-` return `10`.
- **The Parsing Loop (`binexpr`)**: 
  1. The parser reads an expression (like a number) and gets a left-hand side.
  2. It looks at the next operator (e.g., `+`).
  3. It enters a loop: *While the next operator's precedence is higher than the current precedence level we are operating at, keep parsing recursively.*
  4. Because `*` has a higher precedence than `+`, when it hits `*`, the right-hand side of the `+` recursively triggers another expression parse that "binds" tighter to the surrounding numbers.

## Why this is crucial
Without precedence parsing, building a reliable compiler for any language is impossible. This stage ensures that the mathematical tree we build is robust and mathematically accurate before we ever attempt to convert it to x86 assembly.
