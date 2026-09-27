# 09 — Compilação

Módulo 10: Linguagens e compiladores · Nível: Profundo

## Ideia central

Compilação pode usar IR antes de assembly. IR facilita análise/otimização e separa frontend de backend.

## Modelo mental

Linguagens de programação são sistemas implementados. Lexer, parser, AST, tipos, VM, compilador e GC são mecanismos concretos que podem ser construídos.

Neste capítulo identifique estado, invariante, operação e custo. Se houver uma abstração, pergunte qual problema ela resolve, que garantias oferece e o que acontece uma camada abaixo.

## Mecanismo passo a passo

1. Represente a entrada: quais objetos, bits, nós, mensagens, registros ou estados existem antes da operação.
2. Aplique a regra de transição: qual informação é lida, qual condição é testada e qual estado é modificado.
3. Preserve a invariante: qual propriedade precisa continuar verdadeira após cada passo.
4. Produza uma observação: resultado, saída, novo estado, mensagem ou efeito persistido precisa ser verificável.
5. Analise crescimento e falha: o que muda com escala, interrupção no meio e entrada adversa.

Esse procedimento transforma a definição de Compilação em uma máquina mental simulável, testável e depurável.

## Código real do módulo

Leia o arquivo principal [projetos/mini-lang/interpreter.py](../projetos/mini-lang/interpreter.py). Ele faz parte da suíte executável do curso.

Para validar o laboratório:

~~~bash
cd 10-linguagens-e-compiladores/projetos/mini-lang
python -m unittest -v
~~~

Faça uma leitura em três passagens: primeiro encontre entrada, saída e estado persistente; depois marque onde a invariante é criada e atualizada; por fim encontre o caminho de erro e um caso extremo coberto por teste.

## Trade-offs

Interpretação favorece simplicidade e dinamismo; compilação e bytecode podem melhorar desempenho e análise ao custo de pipeline mais complexo.

Para Compilação, separe custo assintótico de custo concreto. CPU, memória, I/O, coordenação e complexidade operacional são recursos diferentes; melhorar um pode piorar outro.

## Erros comuns

Procure ativamente por: precedência errada, escopo dinâmico acidental, stack da VM inconsistente, diagnóstico sem posição e otimização sem prova.

Um bom teste não cobre apenas o caso feliz. Escreva ao menos um teste que viole uma pré-condição e outro que pressione um limite de tamanho, ordem, concorrência ou persistência.

## Experimento guiado

1. Escolha uma entrada pequena simulável à mão.
2. Registre o estado relevante antes de cada passo.
3. Preveja o resultado e só então execute o laboratório.
4. Instrumente uma variável, contador, endereço, fila, árvore ou mensagem.
5. Crie um caso extremo: vazio, limite, repetido, inválido, desordenado ou grande.
6. Reduza divergências até localizar a primeira transição inesperada.
7. Transforme a descoberta em teste automatizado.

## Conexões

Este capítulo vem depois de Bytecode e VM e prepara Garbage collection. Identifique qual conceito anterior fornece a representação usada aqui e qual conceito seguinte depende da garantia produzida por este mecanismo.

## Perguntas de domínio

- Qual é a definição operacional de Compilação?
- Que invariante ou garantia é essencial?
- Qual é a entrada e qual estado é modificado?
- Que custo de tempo, espaço, coordenação ou I/O cresce com a entrada?
- O que a abstração esconde da camada inferior?
- Que falha aparece se uma hipótese central deixar de valer?
- Como demonstrar a propriedade com teste e, quando necessário, com prova?

## Exercícios

1. Explique o conceito em cinco frases sem consultar a aula.
2. Crie um exemplo correto e um contraexemplo.
3. Desenhe o estado antes e depois de uma operação.
4. Torne o conceito observável no laboratório.
5. Escreva um teste de caso extremo ainda inexistente.
6. Compare duas alternativas e explicite qual recurso cada uma otimiza.
7. Conecte este capítulo ao anterior e ao próximo.

## Critério de conclusão

Você concluiu quando consegue prever um caso novo, explicar o mecanismo abaixo da API, localizar a invariante no código, escrever um teste que detecte sua quebra e justificar pelo menos um trade-off.
