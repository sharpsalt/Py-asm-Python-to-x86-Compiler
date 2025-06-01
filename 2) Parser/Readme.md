## Part 2: Parser

In this part of Our Implementation Journey.Parser is one of the important things which contribute to its development. As I mentioned, the main job of the parser is to recognize the Syntax,Structure of the input and ensure that they align correctly with the Grammar of the Language.

Basically in Lexer Part we have Implemented Some Basic Stuffs:
-Four Basic Math Operators: +, -, *, and /.
-Assignment Opertors and Ident for Python
-Decimal Representation(Limited to int) but infuture i will Add float , complex , etc.

Now let's define a grammar for the language that our parser will recognise.


 ## BNF: Backus-Naur Form
 
You’ll eventually encounter BNF (Backus-Naur Form) if you start working with programming languages or language parsers. It’s a formal way of describing the grammar or syntax rules of a language.

In this section, we’ll just introduce enough BNF to describe the simple mathematical expressions we want our compiler to recognize — specifically, expressions involving whole numbers.

```bash
expression: number
          | expression '*' expression
          | expression '/' expression
          | expression '+' expression
          | expression '-' expression
          ;

number:  T_INT
         ;
```

The vertical bars separate options in the grammar, so the above says:
-An expression could be just a number, or
-An expression is two expressions separated by a '*' token, or
-An expression is two expressions separated by a '/' token, or
-An expression is two expressions separated by a '+' token, or
-An expression is two expressions separated by a '-' token
-A number is always a T_INT token

You might have noticed that the BNF grammar is recursive—for example, an expression is defined in terms of other expressions. This means the parser may keep calling itself as it tries to match nested expressions.

However, this recursion eventually "bottoms out" when the expression is just a number. At that point, the number is represented by a T_INT token, which is a single, indivisible unit—not something we need to parse further. So there's no recursion at that level.

In BNF terminology:

    "expression" and "number" are called non-terminal symbols. These are symbols that appear on the left-hand side of grammar rules and are further broken down by other rules.

    On the other hand, T_INT (a literal integer token) is a terminal symbol. It isn’t defined by any further rules—it’s a basic building block of the language, recognized directly by the lexer.

The same goes for the math operators like +, -, *, and /. These are also terminal symbols, since the lexer identifies them directly as tokens.

## Recursive Descent Parsing
Since the grammar for our language is recursive, it makes sense to write a recursive parser. The idea is straightforward: we read in one token, then look ahead to the next token. Based on what we see, we decide what part of the grammar to apply next. This might involve calling a function that calls itself—hence, recursive descent parsing.

In our case, every expression starts with a number. That number may be followed by a math operator (+, -, etc.), and after that, we might either see:

    Just another number, or

    The beginning of a whole new sub-expression.

So how do we handle this with recursion?

We can write some simple pseudo-code for the parser like this:

function expression() {
  Scan and check that the first token is a number.
  If it's not, report a syntax error.

  Get the next token.
  If the token is the end-of-file (T_EOF), return — this is our base case.

  Otherwise, call expression() again.
}

Let’s walk through this using an example input:
2 + 3 - 5 T_EOF
(T_EOF is a special token that marks the end of input.)

We’ll number each call to the expression() function for clarity:

    expression0:

        Scan the 2, it's a number ✔️

        Next token is + — not T_EOF

        Call expression()

            expression1:

                Scan the 3, it's a number ✔️

                Next token is - — not T_EOF

                Call expression()

                    expression2:

                        Scan the 5, it's a number ✔️

                        Next token is T_EOF — base case, return

                Return from expression1

        Return from expression0

So, yes — the recursive function successfully parsed the input: 2 + 3 - 5 T_EOF.

Keep in mind that, so far, this parser only recognizes whether the input follows the grammar. It doesn’t actually do anything with the input (like evaluating the expression). That job is left to semantic analysis, which interprets the meaning.

    (Although you’ll see later that in practice, it’s common to mix syntax analysis and semantic actions together for simplicity and performance.)
