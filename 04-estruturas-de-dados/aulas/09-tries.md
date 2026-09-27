# 09 — Tries

Módulo 04: Estruturas de dados · Nível: Essencial

## Ideia central

Trie percorre símbolos da chave por arestas. Busca custa O(L) e prefixos compartilhados reutilizam a mesma trajetória.

## Modelo mental

Uma estrutura de dados é uma escolha de representação que torna algumas operações baratas e outras caras. O foco é manter invariantes e entender custo temporal e espacial.

Neste capítulo identifique estado, invariante, operação e custo. Se houver uma abstração, pergunte qual problema ela resolve e o que acontece uma camada abaixo.

## Código e laboratório

O laboratório principal do módulo está em [projetos/hashmap-python](../projetos/hashmap-python). Execute os testes antes de alterar o código.

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

- Qual é a definição operacional de Tries?
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
