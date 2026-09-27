# Parsing

O lexer transforma caracteres em tokens. O parser transforma a sequência de tokens em uma estrutura que representa a sintaxe.

Para:

~~~text
2 + 3 * 4
~~~

a árvore desejada é equivalente a:

~~~text
    +
   / \
  2   *
     / \
    3   4
~~~

e não `(2 + 3) * 4`. A gramática codifica precedência e associatividade.

## Recursive descent

Uma função por nível de precedência é uma técnica direta:

~~~text
expression → term (("+" | "-") term)*
term       → unary (("*" | "/") unary)*
unary      → "-" unary | primary
primary    → NUMBER | "(" expression ")"
~~~

`expression()` chama `term()`, que consome multiplicações antes de devolver controle. Assim a precedência emerge da estrutura do parser.

## AST

A Abstract Syntax Tree elimina detalhes puramente sintáticos e preserva relações semânticas necessárias às próximas fases.

Depois do parser, um interpretador pode avaliar a AST diretamente. Um compilador pode transformá-la para uma representação intermediária, otimizar e gerar código.

O projeto `mini-lang` implementa exatamente esse pipeline e é pequeno o suficiente para ser refeito do zero.
