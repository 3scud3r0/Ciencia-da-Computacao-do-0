# B+ Tree

A implementação mantém valores apenas nas folhas e usa chaves internas como separadores. Quando um nó ultrapassa `max_keys`, ele é dividido; o separador sobe para o pai. Se a raiz divide, a altura aumenta.

Todas as folhas permanecem na mesma profundidade. Além disso, folhas possuem ponteiro `next`, permitindo **range scans** sem voltar pela árvore.

O teste insere 1.000 chaves em ordem aleatória, valida invariantes, pesquisa todas elas e verifica ranges. A versão seguinte deve substituir objetos Python por **páginas de tamanho fixo**, adicionando serialização, page IDs e split persistente — o passo que aproxima a estrutura de um storage engine real.
