# 5) Statements and the Print Command

Up until now, our compiler only understood single mathematical expressions. A real language is made of a sequence of **statements**. This directory transitions us from a calculator into a procedural language.

## The Goal of this Stage
To allow the compiler to read multiple lines of code, specifically introducing the ability to parse an explicit command: `print(expression)`.

## Key Concepts Explained

### 1. The `statement()` function (`statements.c`)
We introduce a higher-level parsing concept called a statement. 
- The parser looks at the first token of a line. If it sees the keyword `print`, it shifts into a new parsing mode specifically expecting an opening parenthesis, an expression (calling our Stage 3 parser), and a closing parenthesis.

### 2. The GLUE Node (`A_GLUE`)
How do we represent multiple statements in a single tree? We introduce a structural node called `A_GLUE`. 
- If you have two `print` statements, the root of the AST will be an `A_GLUE` node. 
- Its left child will be the first statement.
- Its right child will be the next statement (which could be another `A_GLUE` node linking more statements down the chain).
- This creates a linked list of operations built out of a rigid tree structure.

### 3. The `A_PRINT` Node
When `print(...)` is successfully parsed, an `A_PRINT` node is created. Its single child is the mathematical expression it needs to print. 

### 4. Updating Code Generation
- The Code Generator (`gen.c`) is updated so that when it hits an `A_GLUE` node, it simply generates code for the left child, and then generates code for the right child sequentially.
- When it hits an `A_PRINT` node, it generates the math for the child expression, leaves the result in a register, and then emits assembly to call an external C function (`printint`) to print that register out to the console.
