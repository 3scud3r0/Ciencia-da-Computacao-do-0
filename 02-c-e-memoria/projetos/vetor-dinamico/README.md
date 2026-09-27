# Vetor dinâmico em C

Um `list` parece crescer sozinho; este projeto desmonta a abstração.

A estrutura guarda `data`, `size` e `capacity`, mantendo a invariante **0 ≤ size ≤ capacity**. Quando fica cheia, a capacidade dobra. Uma realocação custa O(n), mas uma sequência de `push` tem custo amortizado O(1).

O detalhe crítico é usar um temporário com `realloc`: se a alocação falhar, o ponteiro antigo continua recuperável.

Execute `make test`. O alvo compila com AddressSanitizer e UBSan. Depois altere o fator de crescimento e meça realocações.
