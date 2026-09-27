# Consenso distribuído

Consenso aparece quando vários nós precisam concordar sobre uma sequência de decisões apesar de falhas.

Exemplo: três réplicas mantêm um log. Um cliente pede “grave X”. Não basta enviar a todas: mensagens podem atrasar, nós podem cair e líderes podem mudar.

## O núcleo do problema

Precisamos simultaneamente de propriedades como:

- **safety**: nós corretos não decidem valores incompatíveis;
- **liveness**: o sistema consegue avançar quando as condições permitem.

Uma solução que “sempre responde imediatamente” durante qualquer partição provavelmente está sacrificando alguma garantia.

## Raft como modelo pedagógico

Raft organiza o problema em termos de follower, candidate e leader; eleições divididas em termos; e replicação de um log ordenado.

Um líder considera uma entrada committed após obter a confirmação exigida pelo protocolo. Réplicas aplicam entradas committed à state machine na mesma ordem.

## Maioria

Em um cluster de 5 nós, quorum majoritário é 3. Dois quorums de tamanho 3 necessariamente se intersectam, uma propriedade estrutural usada para preservar informação entre decisões.

## O ponto conceitual

Consenso não “faz a rede ficar confiável”. Ele define regras para que um conjunto de máquinas produza uma história coerente mesmo sob falhas previstas pelo modelo.

Antes de implementar Raft, domine clocks, falhas, replicação e consistência.
