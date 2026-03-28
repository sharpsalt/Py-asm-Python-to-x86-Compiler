# 2) Parser (Abstract Syntax Trees)

This directory builds upon the Scanner by introducing the **Parser**. If the Scanner groups letters into "words" (tokens), the Parser organizes those words into grammatically correct "sentences."

## The Goal of this Stage
We need a way to represent the mathematical operations to be performed in a way that respects order of operations. We do this by building an **Abstract Syntax Tree (AST)**.

## Key Concepts Explained

### 1. The Abstract Syntax Tree (`ast.h`)
An AST is a tree data structure where inner nodes are operators (like `+`, `*`) and leaf nodes are operands (like variables or numbers). 
For a simple expression like `2 + 3`, the `+` is the root, `2` is the left child, and `3` is the right child.

### 2. Building the Tree (`expression.c`)
The parser requests tokens from the scanner one by one.
- **Leaves**: When it sees a number token (`T_INT`), it creates a leaf node (`A_INT`) to hold that value.
- **Nodes**: When it sees an operator (`T_PLUS`), it creates an inner node (`A_ADD`). It then recursively parses the left side and the right side to attach as children to this inner node.

### 3. Basic Interpreter (`interpreter.c`)
To verify our AST is built correctly, this stage often includes a simple tree-walking interpreter. It evaluates the tree recursively:
- To evaluate a `+` node, it recursively evaluates the left child, recursively evaluates the right child, and adds the results together.

## Limitations at this Stage
This early parser usually only handles very simple expressions strictly from left to right (or right to left). It does *not* yet understand standard mathematical precedence (e.g., that `*` should happen before `+`). That is tackled in the next directory.
