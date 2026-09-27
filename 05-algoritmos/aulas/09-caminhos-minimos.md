# 09 — Caminhos mínimos

Módulo 05: Algoritmos · Nível: Essencial

## Ideia central

Dijkstra exige pesos não negativos; Bellman-Ford tolera negativos e detecta ciclos negativos; Floyd-Warshall resolve all-pairs em O(V³).

## Modelo mental

Algoritmos transformam problemas em procedimentos e permitem comparar soluções independentemente de uma máquina específica. Correção e complexidade caminham juntas.

Neste capítulo identifique estado, invariante, operação e custo. Se houver uma abstração, pergunte qual problema ela resolve e o que acontece uma camada abaixo.

## Código e laboratório

O laboratório principal do módulo está em [projetos/roteador-dijkstra](../projetos/roteador-dijkstra). Execute os testes antes de alterar o código.

Ciclo de investigação:

1. execute a versão atual;
2. formule uma hipótese;
3. altere uma variável, estrutura ou entrada;
4. observe teste, saída, tempo, memória ou estado;
5. explique causalmente o resultado.

## Experimento guiado

1. Escolha uma entrada pequena simulável à mão.
2. Registre o estado antes de cada passo.
3. Execute e compare com sua previsão.
4. Crie um caso extremo: vazio, limite, repetido, inválido ou grande.
5. Reduza qualquer divergência até o primeiro passo inesperado.

## Perguntas de domínio

- Qual é a definição operacional de Caminhos mínimos?
- Que invariante ou garantia é essencial?
- Que custo de tempo, espaço, coordenação ou I/O cresce com a entrada?
- O que a abstração esconde da camada inferior?
- Que falha aparece se uma hipótese central deixar de valer?

## Exercícios

1. Explique o conceito em cinco frases sem consultar a aula.
2. Crie um exemplo correto e um contraexemplo.
3. Torne o conceito observável no laboratório do módulo.
4. Justifique o resultado com vocabulário técnico.
5. Conecte este capítulo ao anterior e ao próximo.

## Critério de conclusão

Você concluiu quando consegue prever um caso novo, explicar o mecanismo por baixo da API e justificar pelo menos um trade-off relevante.
