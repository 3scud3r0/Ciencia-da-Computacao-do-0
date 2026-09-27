# Índices de banco de dados

Sem índice, encontrar linhas por uma coluna pode exigir varrer todas as páginas da tabela. Um índice mantém uma estrutura adicional que troca espaço e custo de escrita por buscas mais eficientes.

## B+ Tree

B+ Trees são adequadas a storage porque cada nó contém muitas chaves/filhos. Isso reduz a altura e, portanto, a quantidade de páginas lidas.

~~~text
             [20 | 50]
            /    |    \
         ...    ...   ...
              folhas ligadas
~~~

Dados/ponteiros de registro ficam principalmente nas folhas; folhas encadeadas facilitam range scans.

## Por que não uma BST comum?

No disco/SSD, o custo relevante não é apenas número de comparações. Acessar páginas é muito mais caro. Uma árvore com branching factor alto pode localizar bilhões de itens em poucos níveis.

## Índice não é grátis

Cada INSERT/UPDATE/DELETE pode exigir alterações de índice. Muitos índices aumentam write amplification, ocupam cache e podem tornar planejamento mais complexo.

## Índice composto

A ordem das colunas importa. Um índice em `(country, created_at)` organiza primeiro por country e, dentro dele, por created_at. Isso determina quais filtros e ordenações podem usá-lo eficientemente.

A conexão com estruturas de dados é direta: o B+ Tree do módulo 04 reaparece aqui com páginas, persistência, splits e concorrência.
