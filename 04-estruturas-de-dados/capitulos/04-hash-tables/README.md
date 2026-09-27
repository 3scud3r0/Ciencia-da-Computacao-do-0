# Hash tables

Uma hash table tenta transformar uma chave em uma posição de array.

~~~text
chave
  ↓ hash()
inteiro grande
  ↓ redução para capacidade
índice
~~~

Se duas chaves chegam ao mesmo índice, ocorreu uma **colisão**. Colisões são inevitáveis: o conjunto possível de chaves é muito maior que o número de slots.

## Estratégias

**Chaining:** cada bucket guarda uma coleção de entradas.

**Open addressing:** todas as entradas permanecem no array; uma colisão inicia uma busca por outro slot. Linear probing tenta `i, i+1, i+2...`.

## Load factor

`α = número de entradas / número de slots`.

À medida que α cresce, open addressing precisa examinar mais posições. Por isso implementações reais redimensionam antes da tabela ficar cheia.

## Complexidade

Busca, inserção e remoção são **O(1) esperado**, não garantido. Uma distribuição adversa de hashes pode degradar para O(n).

Isso ensina uma distinção importante: notação assintótica depende também de hipóteses sobre entrada e estrutura.

## Tombstones

Em open addressing, remover uma entrada não pode simplesmente marcar o slot como “nunca usado”. Uma busca que dependia daquele caminho pararia cedo demais. Usamos um marcador de “removido” que preserva a cadeia de probing.

Execute `projetos/hashmap-python` e force colisões criando objetos cuja função `__hash__` retorna sempre o mesmo valor.
