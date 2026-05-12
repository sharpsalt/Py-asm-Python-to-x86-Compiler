# Py-ASM: Python to x86-64 Compiler

> **A research-grade, incrementally-built compiler that transforms Python source code into native x86-64 machine code — built from scratch in C(took reference from Tombstone bridge), across 6 progressive stages, with no external compiler frameworks.**

---

## Table of Contents

1. [What This Project Is](#what-this-project-is)
2. [Big Picture Architecture](#big-picture-architecture)
3. [Repository Layout](#repository-layout)
4. [Stage-by-Stage Deep Dive](#stage-by-stage-deep-dive)
   - [Stage 1 — Scanner (Lexer)](#stage-1--scanner-lexer)
   - [Stage 2 — Parser & Tree-Walking Interpreter](#stage-2--parser--tree-walking-interpreter)
   - [Stage 3 — Precedence & First Code Generation](#stage-3--precedence--first-code-generation)
   - [Stage 4 — End-to-End Assembly Pipeline](#stage-4--end-to-end-assembly-pipeline)
   - [Stage 5 — Statements, Print, If/Elif/Else](#stage-5--statements-print-ifelifelse)
   - [Stage 6 — Variables, IR, Register Allocation, Optimizations & Runtime](#stage-6--variables-ir-register-allocation-optimizations--runtime)
5. [Compiler Pipeline in Full Detail](#compiler-pipeline-in-full-detail)
6. [Token System](#token-system)
7. [AST Design](#ast-design)
8. [Intermediate Representation (IR)](#intermediate-representation-ir)
9. [Optimization Passes](#optimization-passes)
10. [Liveness Analysis](#liveness-analysis)
11. [Linear Scan Register Allocator](#linear-scan-register-allocator)
12. [Ownership & Auto-Drop Pass](#ownership--auto-drop-pass)
13. [x86-64 Code Generation Backend](#x86-64-code-generation-backend)
14. [Runtime Library](#runtime-library)
15. [Tagged Pointer Object System](#tagged-pointer-object-system)
16. [Supported Python Features](#supported-python-features)
17. [Building & Running](#building--running)
18. [Test Files](#test-files)
19. [Sample Assembly Output](#sample-assembly-output)
20. [Key Design Decisions](#key-design-decisions)
21. [Known Limitations](#known-limitations)

---

## What This Project Is

Python is typically executed by the CPython interpreter — a virtual machine that reads and evaluates bytecode line-by-line. This project takes a fundamentally different approach: it compiles Python source code **directly into native x86-64 assembly**, bypassing the interpreter entirely.

The compiler is written in **C** and serves as a "tombstone bridge" — each stage in its development represents a concrete compiler theory concept brought to life:

- Lexical analysis (tokenizing raw characters into meaningful tokens)
- Syntactic analysis (Pratt parsing into an Abstract Syntax Tree)
- Semantic analysis (tree-walking interpreter for verification)
- Intermediate Representation (Three-Address Code with Basic Blocks)
- Dataflow Analysis (Liveness intervals across instructions)
- Register Allocation (Linear Scan, with spill support)
- Ownership semantics (compile-time auto-drop injection, inspired by Rust)
- Optimization (7 AST-level passes before IR generation)
- Backend code generation (AT&T-syntax x86-64 assembly, linked into a real ELF binary)

The project was built **incrementally** — each of the 6 stages compiles and runs independently, making it possible to study and understand each layer in isolation.

---

## Big Picture Architecture

```
Python Source (.py)
        │
        ▼
┌─────────────────┐
│   Tokenizer     │  Stage 1 – Lexical Analysis
│  (tokenizer.c)  │  Converts raw characters → Token stream
└────────┬────────┘  Handles: INDENT/DEDENT, strings, numbers,
         │           operators, keywords, comments
         ▼
┌─────────────────┐
│    Parser       │  Stage 2/3 – Syntactic Analysis
│ (expression.c,  │  Pratt (precedence climbing) parser
│  statements.c)  │  Produces Abstract Syntax Tree (AST)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   Optimizer     │  Stage 6 – 6 AST-Level Passes
│  (optimize.c)   │  Constant Folding, DCE, Strength Reduction,
└────────┬────────┘  Algebraic Simplification, Const Propagation, Re-fold
         │
         ▼
┌─────────────────┐
│  IR Generator   │  Stage 6 – Three-Address Code
│    (ir.c)       │  Flattens AST → Linked list of IRInst
└────────┬────────┘  Organized into Basic Blocks & IRFunctions
         │
         ▼
┌─────────────────┐
│ Liveness Anal.  │  Stage 6 – Live Interval Computation
│  (liveness.c)   │  Assigns sequential IDs; computes [start, end]
└────────┬────────┘  per virtual register
         │
         ▼
┌─────────────────┐
│ Ownership Pass  │  Stage 6 – Rust-Style Auto-Drop
│  (liveness.c)   │  Injects IR_FREE after last use of heap objects
└────────┬────────┘  Respects move semantics (skips moved values)
         │
         ▼
┌─────────────────┐
│  Register       │  Stage 6 – Linear Scan Allocation
│  Allocator      │  Maps virtual regs → physical regs (r8–r11)
│  (regalloc.c)   │  Spills to stack when all 4 registers busy
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  x86-64 Backend │  Stage 6 – Final Assembly Generation
│  (ir_gen.c)     │  Emits AT&T-syntax .s file
└────────┬────────┘  System V ABI for function calls
         │
         ▼
┌─────────────────┐
│ Runtime Library │  Stage 6 – C Runtime Support
│  (runtime.c)    │  alloc_string, alloc_list, list_append,
└────────┬────────┘  print_object, free_object
         │
   gcc / as / ld
         │
         ▼
  Native ELF Binary
  (./program)
```

---

## Repository Layout

```
Py-asm-Python-to-x86-Compiler/
├── README.md
│
├── 1) Scanner/                  ← Stage 1: Lexer only
│   ├── token.h                  Token type enum + Token struct
│   ├── tokenizer.h              Lexer interface
│   ├── tokenizer.c              Full lexer implementation (64KB buffered)
│   ├── main.c                   Prints every token from input.py
│   ├── input.py                 Test expression
│   └── compiler                 Pre-built binary
│
├── 2) Parser/                   ← Stage 2: Parser + interpreter
│   ├── ast.h                    AST node constructors
│   ├── expression.c             Pratt parser (binexpr)
│   ├── globals.h / globals.c    Shared state (CurrentToken, Infile)
│   ├── interpreter.h / .c       Tree-walking evaluator (int + float)
│   ├── main.c                   Parse → interpret → print result
│   └── input.py                 Test expression
│
├── 3) Precedence/               ← Stage 3: First code generation
│   ├── (all Stage 2 files +)
│   ├── gen.h / gen.c            AST-to-assembly bridge
│   ├── x86codegenerator.h / .c  Register allocator + instruction emitter
│   ├── printint.c               C helper: void printint(int x)
│   └── input.py                 Empty (generates out.s)
│
├── 4) Assembly/                 ← Stage 4: Full .py → binary
│   ├── (same as Stage 3)
│   └── input.py                 2 + 3 * 5 - 8 / 3
│
├── 5) Statements-Print/         ← Stage 5: Multi-statement + control flow
│   ├── statements.h / .c        print(), if/elif/else, parse_block()
│   ├── (extended expression.c   Adds comparison operators)
│   ├── (extended x86codegen)    Adds cmpq, setX, jmp, labels
│   └── input.py                 if 1<0: ... elif 1==1: ... else: ...
│
└── 6) Variable & Assignment/    ← Stage 6: Full research-grade compiler
    ├── token.h                  Extended: T_IN, T_RANGE, T_BREAK, T_CONTINUE
    ├── ast.h                    Full AST: A_WHILE, A_FUNCTION, A_CALL,
    │                            A_STRING, A_LIST, A_BREAK, A_CONTINUE
    ├── tokenizer.c              Extended tokenizer
    ├── expression.c             Extended Pratt parser (unary -, lists, calls)
    ├── statements.c             Full statement parser (while, for, def, return)
    ├── astone.c                 AST node constructors (with ident field)
    ├── interpreter.h / .c       Full interpreter (globals hash table, functions)
    ├── optimize.h / optimize.c  6 AST optimization passes
    ├── ir.h / ir.c              Three-Address Code IR + Basic Blocks + CFG
    ├── liveness.h / liveness.c  Liveness analysis + Ownership/Auto-Drop pass
    ├── regalloc.h / regalloc.c  Linear Scan Register Allocator
    ├── ir_gen.h / ir_gen.c      x86-64 assembly emitter from IR
    ├── runtime.c                C runtime: tagged pointers, heap objects
    ├── globals.h / globals.c    Shared state
    ├── Makefile                 Build system
    ├── input.py                 Comprehensive test suite
    ├── test_advanced.py         Memory mgmt, optimization, strings, lists
    ├── test_func.py             Functions: add, square, fibonacci, factorial
    ├── test_loops.py            While loops, for-range loops
    ├── test_optimizations.py    Constant folding, strength reduction, propagation
    ├── test_simple.py           Basic variable assignment
    ├── test_aug.py              Augmented assignment (+=, -=, etc.)
    ├── test_break_continue.py   break/continue in loops
    ├── test_heap.py             Heap allocation (strings and lists)
    ├── test_spill.py            Register spilling
    ├── test_div.py              Division edge cases
    ├── test_peephole.py         Peephole optimizations
    ├── out.s                    Last generated assembly
    └── Makefile                 gcc-based build
```

---

## Stage-by-Stage Deep Dive

### Stage 1 — Scanner (Lexer)

**Directory:** `1) Scanner/`

**Purpose:** Convert raw Python source text into a stream of typed tokens. This is the first phase of every compiler — turning characters into meaning.

#### Files

**`token.h`** — Defines the `TokenType` enum with **52 distinct token kinds**:

| Category | Tokens |
|---|---|
| Literals | `T_INT`, `T_FLOAT`, `T_STRING` |
| Identifiers | `T_IDENTIFIER` |
| Arithmetic | `T_PLUS`, `T_MINUS`, `T_STAR`, `T_SLASH`, `T_POWER` |
| Bitwise | `T_LSHIFT`, `T_RSHIFT`, `T_BIT_AND`, `T_BIT_OR`, `T_BIT_XOR`, `T_BIT_NOT` |
| Comparisons | `T_EQ`, `T_NE`, `T_LT`, `T_GT`, `T_LE`, `T_GE` |
| Assignment | `T_ASSIGN`, `T_PLUS_EQ`, `T_MINUS_EQ`, `T_STAR_EQ`, `T_SLASH_EQ`, etc. |
| Delimiters | `T_LPAREN`, `T_RPAREN`, `T_LBRACKET`, `T_RBRACKET`, `T_COMMA`, `T_COLON` |
| Keywords | `T_IF`, `T_ELSE`, `T_WHILE`, `T_FOR`, `T_DEF`, `T_RETURN`, `T_PRINT` |
| Python I/O | `T_INPUT`, `T_INT_INPUT`, `T_FLOAT_INPUT` |
| Indentation | `T_NEWLINE`, `T_INDENT`, `T_DEDENT` |

The `Token` struct carries `type`, `text` (heap-allocated C string), `intvalue`/`floatvalue` (pre-parsed), plus `line` and `column` for error reporting.

**`tokenizer.c`** — The full lexer. Key design points:

- **64KB read buffer** (`LEXER_BUFFER_SIZE = 1<<16`): The entire file is read into a static buffer upfront using `fread`, avoiding per-character syscall overhead. Chunked re-filling handles files larger than 64KB.
- **`LexerState` struct**: holds `source`, `buffer[64KB]`, `pos`, `len`, `line`, `column`, an `indent_stack[32]` for Python indentation, and `indent_top`.
- **`next_char()` / `backup_char()`**: inline functions for single-character lookahead. Backup is needed for multi-character tokens (`==`, `+=`, `**`, `<<=`, etc.).
- **Indentation tracking**: Python's significant whitespace is tracked via a stack of indent levels. When a line starts at a greater indent than the top of the stack, a `T_INDENT` is emitted. When indent decreases, `T_DEDENT` is emitted (one per level). On EOF, remaining `T_DEDENT`s are flushed.
- **Special compound-keyword recognition**: `int(input())` and `float(input())` are recognized as single tokens (`T_INT_INPUT`, `T_FLOAT_INPUT`) via recursive peeking: when the lexer sees `int`, it peeks for `(`, then calls `get_next_token()` recursively to check for `T_INPUT`, then expects `)`.
- **String literals**: handles both `'` and `"` delimiters, with escape sequences (`\n`, `\t`, `\r`, `\\`, `\'`, `\"`).
- **Number literals**: distinguishes integers from floats (via `.` detection). Numeric values are pre-parsed into `intvalue` / `floatvalue`.

**`main.c`** — Opens `input.py`, calls `init_lexer()`, and loops calling `get_next_token()`, printing each token's type, line, column, text, and numeric value until `T_EOF`.

**Test input (`input.py`):** `y = (3 + 5) * 10 - 20 / 4`

---

### Stage 2 — Parser & Tree-Walking Interpreter

**Directory:** `2) Parser/`

**Purpose:** Parse the token stream into an **Abstract Syntax Tree (AST)** and evaluate it with a recursive tree-walking interpreter. This stage introduces the core data structure of the whole compiler.

#### New Files Over Stage 1

**`globals.h` / `globals.c`** — Shared mutable state:
- `Token CurrentToken` — the token currently being parsed
- `FILE *Infile` — the source file
- `int Line`, `int Putback` — legacy line tracking (superseded by token.line)

These globals are declared `extern` in `globals.h` and defined once in `globals.c`, following the classic C multi-file pattern.

**`ast.h`** — Declares the `ASTNode` struct (defined inline in `token.h` in later stages, but separated here) with `mkastleaf()` and `mkastnode()` constructors. The AST node enum at this stage: `A_ADD`, `A_SUBTRACT`, `A_MULTIPLY`, `A_DIVIDE`, `A_INT`, `A_FLOAT`.

**`expression.c`** — Implements the **Pratt parser** (also called precedence climbing):

```c
struct ASTNode *binexpr(int ptp) {
    left = primary();
    while (op_precedence(CurrentToken) > ptp) {
        next_token();
        right = binexpr(tokprec);      // recurse with current precedence
        left = mkastnode(arithop(...), left, right, ...);
    }
    return left;
}
```

- `primary()` handles leaf nodes: integer literals, float literals, and parenthesized sub-expressions.
- `op_precedence()` returns `10` for `+`/`-` and `20` for `*`/`/`, enforcing correct operator precedence without grammar transformations.
- `arithop()` maps `TokenType` → AST node type.

This is the key insight of Pratt parsing: instead of grammar rules like `expr → term + expr`, you assign numeric precedences and let the parser climb automatically.

**`interpreter.h` / `interpreter.c`** — Tree-walking evaluator:
- Uses a `union Value { int intval; float floatval; }` for results.
- Recursively evaluates left and right subtrees.
- Promotes to float if either operand is float.
- Detects and reports division by zero.
- Prints intermediate computation steps for debugging.

**Test input (`input.py`):** `100/1+1e3`

---

### Stage 3 — Precedence & First Code Generation

**Directory:** `3) Precedence/`

**Purpose:** Emit **real x86-64 assembly** from the AST. This is the moment the project crosses from interpreter to compiler.

#### New Files Over Stage 2

**`gen.h` / `gen.c`** — The bridge between the AST and the assembly emitter:

```c
static int genAST(struct ASTNode *n) {
    int leftregister, rightregister;
    if (n->left)  leftregister  = genAST(n->left);
    if (n->right) rightregister = genAST(n->right);
    switch (n->op) {
        case A_ADD:      return cgadd(leftregister, rightregister);
        case A_SUBTRACT: return cgsub(leftregister, rightregister);
        case A_MULTIPLY: return cgmul(leftregister, rightregister);
        case A_DIVIDE:   return cgdiv(leftregister, rightregister);
        case A_INT:      return cgload(n->intvalue);
        case A_FLOAT:    return cgload(n->floatvalue);
    }
}
```

The pattern: recursively generate code for sub-trees, then combine results. Each call returns a **register number** (0–3).

**`x86codegenerator.c`** — The actual assembly emitter:

- **Register pool**: 4 scratch registers — `%r8`, `%r9`, `%r10`, `%r11` — tracked with a `freereg[4]` array (1=free, 0=in-use).
- **`cgload(value)`**: allocates a free register, emits `movq $value, %rN`.
- **`cgadd(r1, r2)`**: emits `addq %r1, %r2`, frees r1, returns r2.
- **`cgsub(r1, r2)`**: emits `subq %r2, %r1`, frees r2, returns r1 (note operand order: AT&T syntax subtracts src from dst).
- **`cgmul(r1, r2)`**: emits `imulq %r1, %r2`, frees r1, returns r2.
- **`cgdiv(r1, r2)`**: uses x86's `idivq` which requires `%rax` / `cqo` sign-extension — moves dividend into `%rax`, sign-extends to `%rdx:%rax` via `cqo`, divides, moves quotient back into a scratch register.
- **`cgpreamble()`**: emits `.text`, `.globl main`, `main:` label.
- **`cgpostamble()`**: emits `movq $0, %rdi` / `call exit`.
- **`cgprintint(r)`**: moves result register into `%rdi` and calls the C `printint()` helper.

**`printint.c`** — A tiny C file compiled alongside the generated assembly: `void printint(int x) { printf("%d\n", x); }`. This allows the assembly to call into C's `printf` without implementing printf in assembly.

**`globals.h`** — Now adds `extern FILE *Outfile` — the output `.s` file pointer.

**`astone.c`** — Moves AST node allocation into its own file; adds `mkastunary()` for single-child nodes.

The interpreter (`interpreter.c`) is upgraded to use a proper `struct EvalResult { enum ValueType type; union Value v; }` instead of relying on the root node type to infer float/int.

---

### Stage 4 — End-to-End Assembly Pipeline

**Directory:** `4) Assembly/`

**Purpose:** The first stage that produces a working native binary from a `.py` file. Files are essentially identical to Stage 3 but `main.c` ties together the full pipeline:

1. Open `input.py`
2. Parse → AST
3. Interpret (for verification / debug output)
4. Generate assembly → `out.s`
5. Call `gcc -o program out.s printint.c` via `system()`
6. Run `./program` via `system()`

**Test input (`input.py`):** `2 + 3 * 5 - 8 / 3`

---

### Stage 5 — Statements, Print, If/Elif/Else

**Directory:** `5) Statements-Print/`

**Purpose:** Extend from single-expression programs to multi-statement programs with control flow. This stage introduces the statement parser, comparison operators, and conditional jumps in generated assembly.

#### New Files Over Stage 4

**`statements.h` / `statements.c`** — The statement-level parser:

- **`parse_statements(FILE*)`**: Top-level entry point. Initializes the lexer, skips leading newlines, then delegates to `statements()`.
- **`statements()`**: Recursively parses a list of statements, stitching them together with `A_GLUE` AST nodes (a binary tree of statements).
- **`statement()`**: Dispatches on current token: `T_PRINT` → `print_statement()`, `T_IF` → `if_statement()`.
- **`print_statement()`**: Matches `print(`, parses an expression with `binexpr(0)`, matches `)`, wraps in `mkastunary(A_PRINT, expr)`.
- **`if_statement()`**: Parses `if condition:`, then `parse_block()` for the body, then optionally chains `parse_elif_or_else()`.
- **`parse_elif_or_else()`**: Recursive. Handles `elif cond: body` chains, building nested `A_IF` AST nodes. Handles `else: body` as the base case.
- **`parse_block()`**: Matches `T_INDENT`, parses statements until `T_DEDENT`. Handles multiple statements within a block via `A_GLUE`.

**`token.h` (extended)** — Adds `T_ELIF` keyword and inlines the `ASTNode` struct definition. Adds comparison AST types: `A_EQ`, `A_NE`, `A_LT`, `A_GT`, `A_LE`, `A_GE`. Also adds `A_PRINT`, `A_GLUE`, `A_IF`.

**`expression.c` (extended)** — Adds comparison operators to `op_precedence()` (returning 5, lower than arithmetic) and to `arithop()`. Now `3 < 5 + 2` correctly parses as `3 < (5 + 2)`.

**`x86codegenerator.c` (extended)** — New instructions for control flow:

- **`cgcompare_and_set(r1, r2, how)`**: Emits `cmpq %r2, %r1`, then a `set*` instruction (`sete`, `setne`, `setl`, `setg`, `setle`, `setge`) into the byte register (`%r8b`, etc.), then `movzbq` to zero-extend the byte result to 64 bits. Returns the register with 0 or 1.
- **`cgcompare_and_jump(op, r, label)`**: Emits `cmpq $0, %rN` then a conditional jump (`je`, `jne`, `jl`, `jg`, `jle`, `jge`) to label `L<N>`.
- **`cglabel(l)`**: Emits `L<N>:`.
- **`cgjump(l)`**: Emits `jmp L<N>`.
- **`breglist[4]`**: A parallel array of byte-register names (`%r8b`, `%r9b`, `%r10b`, `%r11b`) for the `setX` instructions.

**`gen.c` (extended)** — Handles `A_PRINT`, `A_IF`, `A_GLUE`:
- `A_IF`: generates condition code, emits conditional jump to false label, generates if-body, emits unconditional jump past else, generates else-body.
- `A_GLUE`: generates left subtree, then right subtree (sequential execution).
- `A_PRINT`: generates the expression, calls `cgprintint()`.

**`interpreter.c` (extended)** — Handles all new AST node types: `A_PRINT` (evaluates and prints), `A_IF` (evaluates condition, picks branch), `A_GLUE` (evaluates both sides sequentially).

**`main.c` (extended)** — Prints phase labels for each of the 5 compilation steps.

**Test input (`input.py`):**
```python
if 1<0:
    print(10)
elif 1==1:
    print(100)
else:
    print(10000)
```

---

### Stage 6 — Variables, IR, Register Allocation, Optimizations & Runtime

**Directory:** `6) Variable & Assignment/`

**Purpose:** The full research-grade compiler. This stage introduces: variables and assignment, functions with recursion, loops (while + for-range), strings, lists, a Three-Address Code IR, liveness analysis, a linear scan register allocator with spill support, 6 AST optimization passes, an ownership/auto-drop pass, a tagged pointer object system, and a C runtime library.

This is where the project leaps from a toy to a serious compiler infrastructure.

#### Complete File Inventory

| File | Size | Role |
|---|---|---|
| `token.h` | ~80 lines | Extended token types (T_IN, T_RANGE, T_BREAK, T_CONTINUE) |
| `ast.h` | ~80 lines | Full AST node enum, EvalResult, ASTNode with ident field |
| `tokenizer.c` | ~500 lines | Extended lexer (break, continue, in, range) |
| `expression.c` | ~200 lines | Extended Pratt parser (unary minus, lists, calls, identifiers) |
| `statements.c` | 407 lines | Full statement parser (while, for, def, return, augmented assign) |
| `interpreter.c` | ~500 lines | Full interpreter (hash-table globals, function table, closures) |
| `optimize.c` | 422 lines | 6 AST optimization passes |
| `ir.c` | 493 lines | IR data structures, emitter, AST→IR translation |
| `liveness.c` | ~150 lines | Liveness analysis + auto-drop injection |
| `regalloc.c` | ~150 lines | Linear scan register allocation with spill |
| `ir_gen.c` | 318 lines | x86-64 assembly emitter from IR |
| `runtime.c` | ~120 lines | Tagged pointers, heap objects, C runtime functions |
| `astone.c` | ~50 lines | AST node constructors |
| `globals.c/h` | ~20 lines | Shared state |
| `Makefile` | 12 lines | Build system |

---

## Compiler Pipeline in Full Detail

Stage 6's `main.c` drives the entire pipeline sequentially:

```
Step 1: parse_statements(input)        → struct ASTNode *ast
Step 2: optimize(ast)                  → ast (mutated in place, 6 passes)
Step 2.5: init_ir(); generate_ir(ast)  → IRFunction linked list
Step 2.6: compute_liveness()           → LiveInterval[] per IRFunction
Step 2.7: insert_auto_drops()          → IR_FREE nodes injected
        : compute_liveness()            → recomputed after drops
        : allocate_registers()          → physical_reg assigned per vreg
Step 4: generate_code_from_ir()        → out.s written
Step 5: gcc -o program out.s runtime.c → native ELF binary
Step 6: ./program                      → execution output
```

---

## Token System

**File:** `6) Variable & Assignment/token.h`

The token enum in Stage 6 contains **55 token types**. The key additions over Stage 1:

```c
T_ELIF,         // elif keyword
T_IN,           // in  (for x in range(...))
T_RANGE,        // range (recognized as a keyword)
T_BREAK,        // break
T_CONTINUE,     // continue
```

The `Token` struct:
```c
typedef struct {
    TokenType type;
    char *text;       // heap-allocated text (caller must free)
    int intvalue;     // pre-parsed integer value
    float floatvalue; // pre-parsed float value
    int line;         // source line (1-based)
    int column;       // source column (1-based)
} Token;
```

The `union Value` for interpreter results:
```c
union Value {
    int intval;
    float floatval;
};
```

---

## AST Design

**File:** `6) Variable & Assignment/ast.h`

### Node Types

```c
enum {
    // Arithmetic
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
    // Comparisons
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
    // Literals
    A_INT, A_FLOAT,
    // Variables
    A_IDENT,    // identifier reference — stores name in ident field
    A_ASSIGN,   // assignment — left=ident node, right=expr
    // Statements
    A_PRINT,    // print statement — left=expr
    A_IF,       // if — left=cond, right=A_GLUE(body, else_body)
    A_GLUE,     // sequential execution — left=stmt1, right=stmt2
    A_WHILE,    // while — left=cond, right=body
    A_NEGATE,   // unary minus — left=expr
    // Functions
    A_FUNCTION, // def — ident=name, left=params, right=body
    A_RETURN,   // return — left=expr
    A_CALL,     // call — ident=name, left=args (A_GLUE tree)
    // Heap types
    A_STRING,   // string literal — ident=string content
    A_LIST,     // list literal — left=elements (A_GLUE tree)
    // Control flow
    A_BREAK,
    A_CONTINUE
};
```

### ASTNode Structure

```c
struct ASTNode {
    int op;               // Node type (from enum above)
    struct ASTNode *left;
    struct ASTNode *right;
    union {
        int intvalue;     // For A_INT
        float floatvalue; // For A_FLOAT
        char *ident;      // For A_IDENT, A_FUNCTION, A_CALL, A_STRING
    };
    int line;
    int column;
};
```

### Constructors (astone.c)

```c
// Two-child binary node
struct ASTNode *mkastnode(int op, struct ASTNode *left, struct ASTNode *right,
                          int intvalue, float floatvalue, const char *ident,
                          int line, int column);

// Leaf (no children)
struct ASTNode *mkastleaf(int op, int intvalue, float floatvalue,
                          const char *ident, int line, int column);

// One-child unary node (uses left child only)
struct ASTNode *mkastunary(int op, struct ASTNode *left,
                           int intvalue, float floatvalue, const char *ident,
                           int line, int column);
```

---

## Intermediate Representation (IR)

**Files:** `ir.h`, `ir.c`

The IR is a **Three-Address Code (TAC)** representation — each instruction has at most one operator and three operands (destination, source1, source2). This is the same fundamental IR design used by GCC (GIMPLE), LLVM, and most production compilers.

### Opcodes

```c
typedef enum {
    IR_ADD, IR_SUB, IR_MUL, IR_DIV,           // Arithmetic
    IR_EQ, IR_NE, IR_LT, IR_GT, IR_LE, IR_GE, // Comparisons
    IR_ASSIGN,      // dst = src1
    IR_JMP,         // unconditional jump to label
    IR_JMP_FALSE,   // jump if src1 == 0
    IR_LABEL,       // label definition
    IR_CALL,        // dst = call src1 (name), src2 (arg count)
    IR_ARG,         // argument setup: arg src1 at position src2
    IR_PARAM,       // parameter receive: dst = param[src1]
    IR_RETURN,      // return src1
    IR_PRINT,       // print src1 (calls print_object at runtime)
    IR_ALLOC_STR,   // dst = alloc_string(src1)
    IR_ALLOC_LIST,  // dst = alloc_list()
    IR_LIST_APPEND, // list_append(src1, src2)
    IR_FREE,        // free_object(src1)   ← injected by ownership pass
    IR_FUNC_BEGIN,  // function prologue marker
    IR_FUNC_END     // function epilogue marker
} IROp;
```

### Operand Types

```c
typedef enum {
    OP_VREG,    // virtual register (t0, t1, t2, ...)
    OP_GLOBAL,  // named variable (x, y, name, ...)
    OP_LOCAL,   // local / parameter
    OP_IMM,     // integer constant
    OP_LABEL,   // jump target label ID
    OP_NONE     // absent operand
} OperandType;
```

### Data Structures

```c
// Single instruction
typedef struct IRInst {
    int id;           // sequential ID for liveness analysis
    IROp op;
    IROperand dst, src1, src2;
    struct IRInst *next, *prev;  // doubly-linked list
} IRInst;

// Basic Block (node in CFG)
typedef struct BasicBlock {
    int id;
    IRInst *head, *tail;
    struct BasicBlock *true_edge;   // fallthrough / jump target
    struct BasicBlock *false_edge;  // conditional jump else-target
    struct BasicBlock *next;        // linked list within function
} BasicBlock;

// Function context
typedef struct IRFunction {
    char *name;
    BasicBlock *entry_block, *exit_block;
    int vreg_count;
    int label_count;
    char *params[4];
    int param_count;
    LiveInterval *intervals;   // allocated by liveness pass
    struct IRFunction *next;
} IRFunction;

extern IRFunction *func_list_head;  // global linked list of all functions
```

### IR Generation (`ir.c`)

The IR generator is a two-function recursive traversal of the AST:

- **`gen_expr(ASTNode *n)`** → `IROperand`: Emits instructions for expressions, returns the operand holding the result (vreg, imm, or global).
- **`gen_stmt(ASTNode *n)`** → `void`: Emits instructions for statements (assign, print, if, while, for, return, function def, call).

Key patterns:
- **Literals** (`A_INT`): returns `new_imm(n->intvalue)` — no instruction emitted.
- **Identifiers** (`A_IDENT`): returns `new_global(n->ident)` — no instruction emitted (loaded lazily in the backend).
- **Binary ops**: emit `IR_ADD/SUB/MUL/DIV/EQ/...` with a fresh `new_vreg()` as destination.
- **Function calls** (`A_CALL`): collects argument nodes into a flat array via `collect_args_ir()`, emits `IR_ARG` for each, then `IR_CALL`.
- **While loops**: emits a start label, condition expression, `IR_JMP_FALSE` to end label, body, `IR_JMP` back to start label, then end label.
- **For loops**: desugared into a while loop over a range counter.
- **Break/Continue**: jump to `current_loop_end` / `current_loop_start` labels, which are saved in globals when entering a while loop.
- **Functions** (`A_FUNCTION`): Creates a new `IRFunction`, pushes it as `current_func`, emits `IR_PARAM` for each parameter, recursively generates the body.

---

## Optimization Passes

**Files:** `optimize.h`, `optimize.c`

Optimizations run **on the AST** (before IR generation), making them source-level transformations. All passes are pure recursive tree traversals (bottom-up, post-order). The `optimize()` function chains them in sequence.

### Pass 1 — Constant Folding

Evaluates compile-time-known expressions and replaces the subtree with a literal node.

```
2 + 3 * 4   →   14        (single movq, no arithmetic at runtime)
5 == 5      →   1         (condition known at compile time)
-10         →   A_INT(-10)
```

Also handles identity/annihilator rules:
- `x + 0 → x`, `0 + x → x`
- `x - 0 → x`
- `x * 1 → x`, `1 * x → x`
- `x * 0 → 0`, `0 * x → 0`

### Pass 2 — Dead Code Elimination

Removes branches whose condition is a constant integer.

```python
if 0:
    print(999)   # → entire if removed
else:
    print(1)     # → replaced by just print(1)

if 1:
    print(42)    # → replaced by just print(42)
```

Detects `A_IF` nodes where `left` is `A_INT(0)` (always false) or `A_INT(nonzero)` (always true).

### Pass 3 — Strength Reduction

Replaces expensive operations with cheaper equivalents:

```
x * 2   →   x + x          (avoids imulq; addition is ~1 cycle vs 3)
x * 8   →   x << 3         (shlq is 1 cycle vs imulq's 3)
x / 4   →   x >> 2         (shrq vs idivq which takes ~20–80 cycles)
```

Uses `is_power_of_two()` and `log2_of()` helpers to detect powers of 2.

### Pass 4 — Algebraic Simplification

Eliminates self-canceling operations:

```
x - x   →   0       (both sides are same A_IDENT node)
x / x   →   1
x + x   →   x * 2   (then caught by Pass 3)
```

Compares identifier names via `strcmp` to detect same-variable references.

### Pass 5 — Constant Propagation

Tracks variables known to hold constant values in a compile-time symbol table (`const_table[]`).

```python
d = 10          # records d=10 in table
e = d + 7       # replaces d with 10 → e = 10 + 7
print(e)        # Pass 6 re-folds 10+7 → 17, print(17)
```

Safety rules:
- Inside `A_WHILE` bodies: invalidates all known constants (loop variables change).
- Inside `A_IF` branches: invalidates all after the condition (both branches may assign differently).
- On any non-constant assignment: invalidates the variable's entry.

### Pass 6 — Re-fold

Re-runs Pass 1 (constant folding) after constant propagation to catch newly constant expressions created by substitution.

---

## Liveness Analysis

**Files:** `liveness.h`, `liveness.c`

Liveness analysis determines for each virtual register the range of instructions during which it is "live" (may be needed). This information is essential for register allocation.

### Algorithm

1. Allocate a `LiveInterval[]` array with one entry per virtual register in the function.
2. Assign sequential integer IDs to every `IRInst` across all basic blocks (linear scan order).
3. For each instruction, scan `src1`, `src2`, `dst`:
   - First time a vreg appears → `intervals[vreg].start = inst.id`
   - Every time a vreg appears → `intervals[vreg].end = inst.id` (updated to latest)

The result: `intervals[v].start` is the first instruction using `v`, `intervals[v].end` is the last.

---

## Linear Scan Register Allocator

**Files:** `regalloc.h`, `regalloc.c`

The classic Poletto & Sarkar (1999) linear scan algorithm, allocating 4 physical hardware registers (`%r8`=0, `%r9`=1, `%r10`=2, `%r11`=3).

### Algorithm

```
1. Sort all live intervals by start point (qsort)
2. For each interval in sorted order:
   a. Expire intervals whose end ≤ current.start (free their registers)
   b. If a free register exists: assign it
   c. If no free register:
      - Find the active interval with the LATEST end point
      - If that interval ends after current: spill it, give current its register
      - Else: spill current (assign to stack slot)
3. Assign stack offsets to spilled vregs (starting at -512(%rbp), growing downward in -8 steps)
```

### Output

Each `LiveInterval` gets either:
- `physical_reg = 0..3` → mapped to `%r8`..`%r11`
- `physical_reg = -1` + `stack_offset = -512, -520, ...` → spilled to stack

The backend (`ir_gen.c`) checks `physical_reg`:
- If `>= 0`: uses the hardware register directly.
- If `< 0`: generates `N(%rbp)` memory operands instead.

---

## Ownership & Auto-Drop Pass

**Files:** `liveness.c` — `insert_auto_drops()`

Inspired by Rust's ownership model, this pass automatically injects `IR_FREE` instructions to free heap-allocated objects at the point of their last use — without requiring the programmer to write `free()`.

### Algorithm

For each virtual register `v` in each function:
1. Find the instruction with `id == intervals[v].end` (the last use).
2. Check if `v` was **moved** (semantics: ownership transferred):
   - **Case 1**: `v` was assigned to a named global (`IR_ASSIGN` with `src1=OP_VREG(v)` and `dst=OP_GLOBAL`) → the global now owns it; skip drop.
   - **Case 2**: `v` was passed to `IR_LIST_APPEND` as the element (`src2`) → the list owns it; skip drop.
   - **Case 3**: `v` is the list in `IR_LIST_APPEND` (`src1`) → it's still alive after; skip drop.
3. If not moved: inject an `IR_FREE` instruction immediately after the last-use instruction.

This means strings and lists are automatically freed when they go out of use — no garbage collector, no reference counting, deterministic compile-time memory management.

---

## x86-64 Code Generation Backend

**File:** `ir_gen.c`

The final pass: walks the IR and emits AT&T-syntax x86-64 assembly into `out.s`.

### ABI Conventions

- **Function prologue**: `pushq %rbp` / `movq %rsp, %rbp` / `subq $N, %rsp`
- **Function epilogue**: `movq %rbp, %rsp` / `popq %rbp` / `ret`
- **Argument registers** (System V x86-64 ABI): `%rdi`, `%rsi`, `%rdx`, `%rcx`
- **Return value**: `%rax`
- **Scratch registers** (callee-saved within our scheme): `%r8`–`%r11`

### Variable Storage

Named variables (globals in Python scope = locals per function in assembly) are stored as stack slots relative to `%rbp`. A simple linear symbol table (`sym_names[]`, `sym_offsets[]`) maps names to offsets. The first variable gets `-8(%rbp)`, the second `-16(%rbp)`, etc.

```c
static int get_or_create_offset(const char *name) {
    // Linear search; if not found, assign next_offset and decrement by 8
}
```

### Stack Frame Layout

```
%rbp + 0   = saved old %rbp
%rbp - 8   = first variable
%rbp - 16  = second variable
...
%rbp - N*8 = Nth variable
%rbp - 512 = first spilled register
%rbp - 520 = second spilled register
...
(total stack size = N*8 + 1024, rounded up to 16-byte alignment)
```

### Key Instruction Patterns

**Tagged integer load:**
```asm
movq $<val*2>, %rax       ; Tagged integers: left-shifted by 1 (LSB=0)
movq %rax, -8(%rbp)
```

**Print (calls runtime):**
```asm
movq <src>, %rdi          ; argument to print_object
pushq %r8..%r11           ; save scratch registers
call print_object
popq %r11..%r8
```

**String allocation:**
```asm
pushq %r8..%r11
leaq .LC0(%rip), %rdi     ; pointer to string in .data section
call alloc_string
popq %r11..%r8
movq %rax, <dst_reg>
```

**Conditional branch:**
```asm
cmpq $0, <cond_reg>
je   L<false_label>
```

**Function call:**
```asm
movq <arg0>, %rdi
movq <arg1>, %rsi
pushq %r8..%r11
call <function_name>
popq %r11..%r8
movq %rax, <dst_reg>
```

---

## Runtime Library

**File:** `runtime.c`

A ~120-line C file compiled alongside the generated assembly (`gcc -o program out.s runtime.c`). It provides the heap object system and I/O.

### Object Hierarchy

```c
typedef struct { ObjType type; } Object;          // base "class"
typedef struct { Object base; char *data; } StringObj;
typedef struct {
    Object base;
    int length, capacity;
    uint64_t *elements;           // dynamic array of tagged values
} ListObj;
```

### Functions

| Function | Signature | Description |
|---|---|---|
| `alloc_string` | `uint64_t (const char*)` | Malloc a StringObj, strdup the text, return tagged pointer |
| `alloc_list` | `uint64_t ()` | Malloc a ListObj with capacity 4, return tagged pointer |
| `list_append` | `void (uint64_t list, uint64_t elem)` | Append element (doubles capacity if needed) |
| `print_object` | `void (uint64_t val)` | Prints int, string, or list based on tag bit |
| `free_object` | `void (uint64_t val)` | Recursively frees objects; prints `[Auto-Drop]` message |

---

## Tagged Pointer Object System

**Inspired by:** V8 JavaScript engine's Small Integer (Smi) optimization.

The fundamental insight: on x86-64, all malloc'd pointers are 8-byte aligned, so their lowest 3 bits are always 0. This allows encoding type information in those bits without extra memory.

### Encoding

```
If LSB == 0: INTEGER  →  actual value = tagged_value >> 1
If LSB == 1: POINTER  →  actual pointer = tagged_value - 1
```

### Macros

```c
#define IS_INT(val)   (((val) & 1) == 0)
#define IS_OBJ(val)   (((val) & 1) == 1)
#define AS_INT(val)   ((int64_t)(val) >> 1)
#define AS_OBJ(val)   ((Object *)((val) - 1))
#define MAKE_INT(val) ((uint64_t)(val) << 1)
#define MAKE_OBJ(ptr) ((uint64_t)(ptr) | 1)
```

### In Generated Assembly

Integers are always shifted left by 1 before storing/passing:
```asm
movq $10, %rax         ; raw int
movq %rax, -8(%rbp)    ; WRONG — would be tagged as pointer
; Correct:
movq $20, %rax         ; 10 << 1 = 20
movq %rax, -8(%rbp)    ; tagged integer 10
```

This is why the backend emits `imm << 1` for immediate values. Lists and strings come pre-tagged from `alloc_string`/`alloc_list` (which return `MAKE_OBJ(ptr)`).

---

## Supported Python Features

### Stage 6 Language Coverage

| Feature | Status | Notes |
|---|---|---|
| Integer literals |  | Stored as tagged pointers |
| Variables & assignment |  | Stack-allocated per function |
| Augmented assignment (`+=`, `-=`, `*=`, `/=`) |  | Desugared to assignment |
| Arithmetic (`+`, `-`, `*`, `/`) |  | Full operator precedence |
| Unary minus (`-x`) |  | A_NEGATE node |
| Comparisons (`==`, `!=`, `<`, `>`, `<=`, `>=`) |  | Returns tagged 0 or 1 |
| `print()` |  | Calls runtime `print_object` |
| `if` / `elif` / `else` |  | Arbitrary chaining |
| `while` loops |  | With `break` and `continue` |
| `for x in range(n)` |  | Desugared to while |
| `for x in range(start, end)` |  | Desugared to while |
| Function definitions (`def`) |  | Up to 4 parameters |
| Function calls |  | Including recursion |
| `return` |  | Propagates up call chain |
| String literals |  | Heap-allocated via `alloc_string` |
| List literals (`[a, b, c]`) |  | Heap-allocated via `alloc_list` |
| Mixed-type lists (`["hello", 5]`) |  | Via tagged pointer union |
| Recursive functions |  | fibonacci, factorial tested |
| Multiple statements |  | A_GLUE tree |
| Indented blocks |  | INDENT/DEDENT tokens |
| Constant folding |  | At AST level, compile-time |
| Dead code elimination |  | `if 0:` → removed |
| Strength reduction |  | `x*2→x+x`, `x*8→x<<3` |
| Algebraic simplification |  | `x-x→0`, `x/x→1` |
| Constant propagation |  | `d=10; e=d+7 → e=17` |
| Auto memory management |  | Compile-time drop injection |
| Register allocation |  | Linear scan with 4 regs |
| Register spilling |  | To stack at `-512(%rbp)` |

### Not Yet Supported (future stages)

| Feature | Notes |
|---|---|
| `float` arithmetic | Tagged pointer encoding is int-only |
| Classes / objects | No OOP infrastructure |
| List indexing (`lst[i]`) | Runtime indexing not implemented |
| Closures / nested functions | Single-level scope only |
| `import` statements | No module system |
| Exception handling (`try/except`) | No unwinding |
| Dictionary, set | No hash map runtime |
| String concatenation | No `+` for strings |
| Multiple return values | Single return only |

---

## Building & Running

### Prerequisites

- `gcc` (any version supporting C11)
- Linux x86-64 (generated assembly uses Linux System V ABI)
- `make`

### Building Stage 6

```bash
cd "6) Variable & Assignment"
make
```

This compiles all `.c` files into object files and links the `compiler` binary:
```
gcc -Wall -Wextra -g -Wno-unused-parameter -c main.c -o main.o
gcc -Wall -Wextra -g -Wno-unused-parameter -c tokenizer.c -o tokenizer.o
... (all 13 source files)
gcc -Wall -Wextra -g -Wno-unused-parameter -o compiler main.o tokenizer.o ...
```

### Compiling a Python File

```bash
./compiler input.py
```

This prints the full compilation pipeline log and produces:
- `out.s` — the generated x86-64 assembly
- `program` — the native executable

Then run the output:
```bash
./program
```

### Building Earlier Stages

```bash
# Stage 1 — just tokenize
cd "1) Scanner"
gcc -o scanner main.c tokenizer.c
./scanner input.py

# Stage 2 — parse + interpret
cd "2) Parser"
gcc -o parser main.c expression.c interpreter.c astone.c globals.c tokenizer.c
./parser input.py

# Stage 3/4 — compile to x86
cd "4) Assembly"
gcc -o compiler main.c expression.c interpreter.c astone.c globals.c tokenizer.c gen.c x86codegenerator.c printint.c
./compiler input.py
./program
```

### Makefile Targets (Stage 6)

```bash
make           # Build the compiler
make test      # Build and run ./compiler input.py
make clean     # Remove all .o files, compiler, out.s, program
```

---

## Test Files

All test files are in `6) Variable & Assignment/`:

### `input.py` — The Comprehensive Test Suite

Exercises every major feature: variables, while loops, for loops, unary minus, range loops, constant folding, dead code elimination, strength reduction, if/else.

```python
x = 5
y = x + 10
print(y)            # 15

i = 0
while i < 5:
    print(i)        # 0 1 2 3 4
    i = i + 1

z = -10
print(z)            # -10

for j in range(3):
    print(j)        # 0 1 2

print(2 + 3 * 4)    # 14 (folded at compile time)

if 0:
    print(999)      # dead code, eliminated

a = 3
b = a * 8           # strength-reduced to a << 3
print(b)            # 24
```

### `test_func.py` — Functions & Recursion

```python
def add(x, y):    return x + y
def square(n):    return n * n
def fibonacci(n):
    if n == 0: return 0
    if n == 1: return 1
    return fibonacci(n - 1) + fibonacci(n - 2)
def factorial(n):
    if n <= 1: return 1
    return n * factorial(n - 1)

print(add(10, 20))        # 30
print(square(5))          # 25
print(fibonacci(6))       # 8
print(factorial(5))       # 120
```

### `test_loops.py` — While and For-Range Loops

```python
i = 0
while i < 5:
    print(i)       # 0 1 2 3 4
    i = i + 1

total = 0
for x in range(1, 6):
    total = total + x
print(total)       # 15
```

### `test_optimizations.py` — Optimization Passes

```python
a = 10 * 3          # Pass 1: folded to 30
print(a)

b = 15
print(b * 2)        # Pass 3: b + b (strength reduce)

c = 42
print(c - c)        # Pass 4: algebraic simplify → 0

d = 10
e = d + 7           # Pass 5+6: d→10, 10+7→17
print(e)
```

### `test_advanced.py` — Strings, Lists, Auto-Drop

```python
name = "RustOwner"
print(name)          # runtime: alloc_string → print_object → [Auto-Drop]

math = 10 * 3
print(math)          # 30 (constant folded)

if 0:
    print(999)       # dead code eliminated
else:
    print(1)

my_list = [10, 20, 30]
print(my_list)       # [10, 20, 30]
```

---

## Sample Assembly Output

From `out.s` (generated by `./compiler test_simple.py`):

```asm
    .data
    .text
    .globl  main
main:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $1120, %rsp          ; Reserve stack frame

L_BB1:
    movq    $2, %rax             ; tagged integer 1 (1 << 1 = 2)
    movq    %rax, -8(%rbp)       ; x = 1

    movq    $4, %rax             ; tagged integer 2
    movq    %rax, -16(%rbp)      ; y = 2

    movq    $6, %rax             ; tagged integer 3
    movq    %rax, -24(%rbp)      ; ...

    movq    $110, %rax           ; tagged integer 55 (result after optimization)
    movq    %rax, -88(%rbp)

    movq    -88(%rbp), %rdi      ; print argument
    pushq   %r8                  ; save scratch registers
    pushq   %r9
    pushq   %r10
    pushq   %r11
    call    print_object         ; runtime print
    popq    %r11
    popq    %r10
    popq    %r9
    popq    %r8

    movq    $0, %rax             ; return 0
    movq    %rbp, %rsp           ; restore stack
    popq    %rbp
    ret
```

---

## Key Design Decisions

### 1. C as the Implementation Language

C provides direct control over memory layout, pointer arithmetic, and system calls — all essential for a compiler. The lack of runtime overhead in C also means the compiler itself is fast, and the generated code has no hidden dependencies.

### 2. Pratt Parsing (Precedence Climbing)

Rather than a full recursive descent grammar with separate rules for each precedence level, Pratt parsing uses a single loop with numeric precedence values. This makes the parser small, easy to extend, and correct by construction.

### 3. Tagged Pointers for Uniform Value Representation

Instead of a separate type tag alongside every value (requiring wider structures), the LSB of every 64-bit value is used to distinguish integers from heap pointers. This is the same technique used by V8, LuaJIT, and many dynamic language runtimes — it saves memory and makes type checks extremely cheap (a single `and` instruction).

### 4. AST-Level Optimization Before IR Generation

Running optimization passes on the AST (rather than the IR) means the simplifications are done at the highest level of abstraction, before any lowering. This catches the most straightforward cases (constant folding, dead code) with simple recursive tree traversals.

### 5. Linear Scan Register Allocation

The classic Poletto-Sarkar algorithm is a good fit for this compiler: it's O(n) in the number of intervals, produces good register assignments for most code, handles spilling gracefully, and is far simpler to implement than graph coloring.

### 6. Compile-Time Auto-Drop (Rust Ownership Analog)

Rather than adding a garbage collector (complex, unpredictable pauses) or requiring manual `free()` calls (error-prone), the ownership pass statically determines the last use of each heap-allocated virtual register and injects a `free_object` call at exactly that point. This gives deterministic, zero-overhead memory management.

### 7. Incremental Compilation Stages

Each stage is a complete, independent, compilable program. This makes the project simultaneously educational (each concept is isolated) and debuggable (you can compare outputs at each stage).

---

## Known Limitations

- **Float arithmetic is not code-generated**: The tokenizer and interpreter handle floats, but the IR and code generator operate only on integers (via tagged pointers). Float support would require either a separate tag bit value or a boxed float object.
- **Maximum 4 function parameters**: The argument-passing registers array (`arg_regs[4]`) is fixed at 4.
- **Maximum 100 global variables per function**: The symbol table (`sym_names[100]`) is statically sized.
- **Maximum 4 physical registers**: Expanding to use callee-saved registers (`%rbx`, `%r12`–`%r15`) would require proper save/restore in function prologues/epilogues.
- **No garbage collection for named variables**: Auto-drop only applies to temporary vregs. Named global variables (Python-scope variables stored on the stack) are never freed — they live for the entire function duration.
- **Single-file programs only**: No module system or `import`.
- **No error recovery**: Any parse error calls `exit(1)` immediately.
- **Strings are immutable**: No string concatenation or mutation operations.
- **List indexing not supported**: Lists can be created and printed, but `lst[i]` syntax is not parsed.

---

*Built from first principles — from raw characters to native silicon.*
