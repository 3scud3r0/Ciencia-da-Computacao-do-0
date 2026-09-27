# 03 — ML por dentro

Módulo 17: Tópicos avançados · Nível: Opcional

## Ideia central

ML começa com loss e otimização. Gradiente dá direção local e backprop aplica regra da cadeia em DAG de operações.

## Modelo mental

Tópicos avançados reutilizam fundamentos em domínios diferentes: álgebra em gráficos, otimização em ML, estruturas em storage e índices em mecanismos de busca.

Neste capítulo identifique estado, invariante, operação e custo. Se houver uma abstração, pergunte qual problema ela resolve e o que acontece uma camada abaixo.

## Fórmula / propriedade central

theta <- theta - eta * grad L(theta)

## Código e laboratório

O laboratório principal do módulo está em [projetos/raytracer](../projetos/raytracer). Execute os testes antes de alterar o código.

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

- Qual é a definição operacional de ML por dentro?
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
