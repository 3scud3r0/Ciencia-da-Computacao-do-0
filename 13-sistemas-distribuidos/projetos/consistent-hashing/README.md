# Consistent hashing

Com `hash(key) % N`, alterar N remapeia grande parte das chaves. Consistent hashing coloca nós e chaves no mesmo anel; uma chave pertence ao primeiro nó em sentido crescente, com wrap-around no final.

Adicionar um nó transfere apenas intervalos vizinhos. **Nós virtuais** melhoram a uniformidade.

O teste verifica a propriedade estrutural: adicionar um quarto nó move uma fração das chaves, e removê-lo restaura o mapeamento anterior.
