# Grammar

## Syntactic grammar

```ebnf
program        → declaration* EOF ;
declaration    → funcDecl
               | varDecl
               | statement ;
funDecl        → "fun" function;
function       → IDENTIFIER  "(" parameters? ")" ;
parameters     → IDENTIFIER ( "," IDENTIFIER )* ;
varDecl        → "var" IDENTIFIER ( "=" expression)? ";" ;
statement      → exprStmt
               | ifStmt
               | printStmt 
               | whileStmt
               | forStmt
               | returnStmt
               | block;
returnStmt     → "return" expression? ";" ;
ifStmt         → "(" (varDecl | exprStmt | ";") expression? ";" expression? ")" statement ;
whileStmt      → "(" expression ")" statement ;
ifStmt         → "if" "(" expression ")" statement ("else" statement)? ;
block          → "{" declaration* "}" ;
exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;
expression     → comma ;
comma          → assignment (',' assignment)* ;
assignment     → IDENTIFIER "=" assignment
               → conditional
conditional    → logic_or (? logic_or : logic_or)* ;
logic_or       → logic_and ("or" logic_and)* ;
logic_and      → equality ( "and" equality )* ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | call ;
call           → primary ( "(" arguments ")" )* ;
arguments      → expression ( "," expression )* ;
primary        → "true" | "false" | "nil"
               | NUMBER | STRING
               | "(" expression ")"
               | IDENTIFIER ;
```
