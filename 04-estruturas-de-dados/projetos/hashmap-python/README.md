# Hash table com open addressing

Esta implementação expõe função hash, colisão, **linear probing**, tombstones, load factor e resize.

A capacidade é potência de 2; por isso o índice usa `hash(key) & (capacity - 1)`. Um slot removido vira `_TOMBSTONE`, não vazio: torná-lo vazio interromperia a cadeia de probing e poderia ocultar chaves que continuam na tabela.

Execute `python -m unittest -v` e depois crie uma classe cuja função `__hash__` sempre retorna o mesmo número para forçar colisões.
