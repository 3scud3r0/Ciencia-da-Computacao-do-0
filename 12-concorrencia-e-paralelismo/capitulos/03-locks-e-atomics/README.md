# Locks e atomics

Uma race condition ocorre quando o resultado depende de uma interleaving não controlada entre operações concorrentes.

`counter += 1` parece uma ação, mas conceitualmente envolve ler, calcular e escrever. Duas threads podem ler o mesmo valor e uma atualização desaparecer.

## Mutex

Um mutex garante exclusão mútua em uma região crítica:

~~~text
lock
  ler estado
  modificar estado
unlock
~~~

O requisito real não é “usar lock”, mas preservar uma **invariante** compartilhada.

## Atomics

Operações atômicas podem realizar certos updates como uma unidade indivisível observável por outras threads. Elas reduzem a necessidade de mutex em alguns padrões, mas introduzem o tema de **memory ordering**: CPUs e compiladores podem reordenar operações dentro de regras formais.

## Deadlock

Locks podem criar espera circular. Quatro condições clássicas — exclusão mútua, hold-and-wait, ausência de preempção e espera circular — ajudam a analisar o problema.

## Regra prática

Use a abstração mais simples que preserve a invariante. Estruturas lock-free não são automaticamente “melhores”; são significativamente mais difíceis de provar e depurar.

O projeto de thread pool demonstra coordenação com uma fila thread-safe e separação entre submissão e execução.
