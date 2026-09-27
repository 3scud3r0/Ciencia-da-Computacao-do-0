# Roteador com Dijkstra

Dijkstra encontra caminhos mínimos quando todos os pesos são não negativos.

O heap pode conter versões antigas da distância de um vértice. Em vez de tentar diminuir uma chave dentro do heap, inserimos a nova distância e ignoramos entradas obsoletas ao retirá-las. Isso mantém a implementação simples.

Com lista de adjacência + heap binário, a complexidade é O((V + E) log V). Compare depois com Bellman–Ford e explique por que pesos negativos mudam o problema.
