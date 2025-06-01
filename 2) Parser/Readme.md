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
