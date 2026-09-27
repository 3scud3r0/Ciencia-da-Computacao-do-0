# P, NP, NP-hard e NP-complete

Aqui “complexidade” trata de classes de problemas, não apenas de contar loops em um programa específico.

## P

Problemas de decisão resolvíveis em tempo polinomial por um modelo determinístico convencional.

## NP

Problemas cujas soluções candidatas podem ser **verificadas** em tempo polinomial. P ⊆ NP.

O “N” não significa “não polinomial”.

## NP-hard

Um problema é NP-hard quando todo problema em NP pode ser reduzido a ele por uma transformação apropriada em tempo polinomial. Ele não precisa pertencer a NP.

## NP-complete

Problemas que são simultaneamente NP-hard e membros de NP.

## Reduções

Para mostrar que B é pelo menos tão difícil quanto A, transforme instâncias de A em instâncias de B eficientemente. Se uma solução eficiente para B implicaria uma solução eficiente para A, a dificuldade é transferida.

## P versus NP

Não se conhece, em geral, se P = NP. O valor pedagógico não é decorar essa frase, mas aprender a reconhecer quando procurar uma solução exata polinomial pode ser uma expectativa errada e quando aproximação, parametrização, heurísticas ou restrições estruturais são apropriadas.

Não confunda pior caso assintótico com desempenho típico em instâncias reais: ambos importam, mas respondem perguntas diferentes.
