# 4) Assembly Generation

This is where it gets exciting. We stop just interpreting the tree in memory and start outputting actual **x86-64 Assembly code**. This directory is the first true "compiler" step.

## The Goal of this Stage
To traverse our accurately built Abstract Syntax Tree (from Stage 3) and emit `.s` text files containing valid Assembly instructions that Linux/GCC can turn into an executable.

## Key Concepts Explained

### 1. The Code Generator (`x86codegenerator.c`)
This file is responsible for translating abstract concepts into physical CPU concepts.
- **`cgpreamble()` and `cgpostamble()`**: These output the boilerplate assembly required to start an executable (e.g., `.text`, `.globl main`, `main:`, and the setup/teardown of the stack frame) and gracefully `exit` the program at the end.
- **Generating Instructions**: Functions like `cgadd`, `cgmul`, and `cgdiv` take register numbers as input and emit strings like `addq %r9, %r8` to a file.

### 2. Register Allocation (`alloc_register`)
CPUs perform math in registers, not directly in memory. However, a CPU only has a limited number of general-purpose registers (like `%r8`, `%r9`, `%r10`, `%r11`).
- We implement a simple tracker (`freereg` array) to know which registers are currently holding data and which are available.
- When traversing the tree to do math, we allocate a register for the left side, allocate another for the right side, perform the assembly instruction, and then **free** one of the registers because the result collapses into the remaining one.

### 3. The Tree Walker (`gen.c`)
This file marries the AST to the Code Generator. It recursively walks down the AST. When returning *up* the tree, it visits the children, gets the register numbers where their results are stored, and calls the appropriate generation function depending on the node type (`A_ADD` calls `cgadd`).

## Output
By the end of this stage, running the compiler on a math expression will generate a file (like `out.s`). You can then run `cc out.s -o program` to get a native executable that performs the math!
