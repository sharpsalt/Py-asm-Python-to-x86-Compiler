# 6) Variables and Assignment

This is the most complex and powerful stage yet. A language needs state, and state means **Variables**. We introduce identifiers (variable names), the assignment operator (`=`), and memory management on the stack.

## The Goal of this Stage
To parse statements like `x = 10` and `y = x + 5`, store those values in memory, and retrieve them when needed in subsequent calculations.

## Key Concepts Explained

### 1. The Symbol Table (`symtab`)
To keep track of variables, we introduce a **Symbol Table**. This is an array lookup system inside `x86codegenerator.c`.
- When the parser sees `x = 10`, it looks up `x`. If `x` doesn't exist, it adds it to the table.
- The most crucial piece of data in the symbol table is the **offset**. This maps the string variable name "x" to a hard physical memory address.

### 2. Stack Memory (Base Pointer Offsets)
We don't use registers for long-term variable storage (we run out too fast). Instead, we store variables on the CPU's Stack.
- We establish a "stack frame" using the Base Pointer (`%rbp`) in our preamble.
- The symbol table assigns an offset (e.g., `-8`, `-16`, `-24`) to each variable. 
- Thus, the variable `x` might permanently live at the memory address `-8(%rbp)`.

### 3. Assignment (`A_ASSIGN`) and Identifiers (`A_IDENT`)
- **Parsing**: We add logic to recognize an assignment (`x = ...`). The left side is an identifier node (`A_IDENT`), and the right is an expression. We link them with an `A_ASSIGN` node.
- **Code Generation (Store)**: For `A_ASSIGN`, we calculate the right side, get the result in a register, and emit a `movq` instruction to store that register into the correct stack memory address `offset(%rbp)` using `cgstorglob`.
- **Code Generation (Load)**: When we see `A_IDENT` inside math (like `x + 5`), we emit a `movq` instruction to load the value from `-8(%rbp)` back *into* a register so the math operations can use it using `cgloadglob`.

By the end of this stage, the project is a fully functional, albeit limited, compiled programming language capable of math, variables, state, and outputs natively on an x86 Linux machine.
