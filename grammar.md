# Grammar

## Syntactic grammar

```ebnf
program        → statement* EOF ;
statement      → exprStmt
               | printStmt ;
exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;
expression     → comma ;
comma          → conditional (',' conditional)* ;
conditional    → equality (? equality : equality)* ; // above assignment in precedence
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | primary ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
               | "(" expression ")" ;
```
