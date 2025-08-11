# Grammar

## Syntactic grammar

```ebnf
program        → declaration* EOF ;
declaration    → varDecl
               | statement ;
varDecl        → "var" IDENTIFIER ( "=" expression)? ";" ;
statement      → exprStmt
               | ifStmt
               | printStmt 
               | block;
ifStmt         → "if" "(" expression ")" statement ("else" statement)? ;
block          → "{" declaration* "}" ;
exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;
expression     → comma ;
comma          → assignment (',' assignment)* ;
assignment     → IDENTIFIER "=" assignment
               → conditional
conditional    → equality (? equality : equality)* ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | primary ;
primary        → "true" | "false" | "nil"
               | NUMBER | STRING
               | "(" expression ")"
               | IDENTIFIER ;
```
