# Mini linguagem interpretada

Gramática:

~~~text
program    → statement* EOF
statement  → "let" NAME "=" expression | "print" expression | expression
expression → term (("+" | "-") term)*
term       → unary (("*" | "/") unary)*
unary      → "-" unary | primary
primary    → NUMBER | NAME | "(" expression ")"
~~~

A precedência nasce da própria gramática: `term` resolve multiplicação/divisão antes de `expression` combinar soma/subtração.

Pipeline: **texto → tokens → AST → ambiente → avaliação**. A próxima evolução será bytecode + máquina virtual.
