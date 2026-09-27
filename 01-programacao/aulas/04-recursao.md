# 04 — Recursão

Módulo 01: Programação e pensamento computacional · Nível: Essencial

## Ideia central

Recursão reduz um problema a instâncias menores até um caso base. Cada chamada mantém estado lógico; ausência de progresso produz recursão infinita.

## Modelo mental

Imagine uma pilha de tarefas pendentes. Para calcular a soma de 1 até 4, a função precisa saber o resultado da soma até 3; esta precisa da soma até 2; e assim por diante. A chamada com zero devolve zero. Só então cada chamada pendente consegue terminar.

O caso base encerra a cadeia. A chamada recursiva reduz o tamanho do problema. Se o argumento nunca se aproximar do caso base, a execução não termina normalmente.

## Mecanismo passo a passo

Para `soma_recursiva(4)`, as chamadas descem com os argumentos `4 → 3 → 2 → 1 → 0`. A última devolve `0`. Na volta, as respostas são `1`, `3`, `6` e `10`.

Em cada chamada, a relação que deve permanecer verdadeira é: resultado para `n` = `n` + resultado para `n - 1`. Essa é a propriedade que permite provar a correção por indução: o caso `n = 0` vale, e cada caso maior depende de um caso menor já correto.

## Código real do módulo

O arquivo [recursao.py](../exemplos/recursao.py) contém as duas versões abaixo. Leia cada linha antes de executar:

~~~python
def soma_recursiva(n: int) -> int:
    if n < 0:
        raise ValueError("n deve ser não negativo")
    if n == 0:
        return 0
    return n + soma_recursiva(n - 1)


def soma_iterativa(n: int) -> int:
    if n < 0:
        raise ValueError("n deve ser não negativo")
    total = 0
    for valor in range(1, n + 1):
        total += valor
    return total
~~~

Na primeira função, a assinatura aceita um inteiro e promete devolver um inteiro. O primeiro `if` rejeita negativos: sem isso, a recursão afastaria o argumento de zero. O segundo `if` é o caso base. A última linha guarda `n` enquanto calcula a soma menor e depois adiciona os resultados.

Na versão iterativa, `total` começa em zero. `range(1, n + 1)` percorre todos os números de 1 até `n`, inclusive. Cada repetição acrescenta um valor; o `return` devolve o acumulado. O tratamento para negativos é igual para que as duas funções tenham o mesmo contrato.

Para executar os testes [test_recursao.py](../exemplos/test_recursao.py) a partir da raiz do repositório:

~~~bash
python -m unittest discover -s 01-programacao/exemplos -p 'test_*.py' -v
~~~

Os testes verificam entradas de zero a 99 pela fórmula `n × (n + 1) ÷ 2` e confirmam que as duas funções rejeitam números negativos.

## Trade-offs

As duas funções fazem `n` adições, portanto têm tempo O(n). A versão recursiva mantém até `n + 1` chamadas pendentes: usa O(n) espaço na pilha e pode atingir o limite de recursão do Python. A versão iterativa mantém um acumulador e usa O(1) espaço extra. Uma fórmula aritmética calcula a mesma soma em O(1), mas serve menos para demonstrar o mecanismo de recursão.

## Erros comuns

Se o caso base faltar, a função continua chamando a si mesma até produzir `RecursionError`. Se a chamada for `soma_recursiva(n + 1)`, o problema cresce e o caso base nunca é alcançado. Se o caso base devolver `1`, todos os resultados ficam uma unidade acima do esperado. Testar somente `n = 1` não demonstra que a cadeia funciona para vários níveis.

## Experimento guiado

1. Desenhe uma linha por chamada de `soma_recursiva(4)` e anote o `n` recebido.
2. Preveja a ordem das respostas antes de executar o código.
3. Adicione um `print(n)` antes do caso base e outro imediatamente antes do retorno final. Compare entrada e saída das chamadas.
4. Altere temporariamente o caso base para devolver `1`; observe qual teste falha e por quê. Depois desfaça a alteração.
5. Execute `soma_recursiva(900)` e `soma_iterativa(900)`; em seguida aumente o argumento gradualmente. A versão recursiva pode esgotar a pilha, dependendo do ambiente.

## Conexões

Funções e escopo explicam por que cada chamada tem seus próprios parâmetros e estado local. A recursão reaparece em árvores, busca em profundidade, divisão e conquista e parsers; nesses casos, a estrutura do problema costuma ser naturalmente recursiva.

## Perguntas de domínio

- Qual condição garante que `soma_recursiva` termine para qualquer inteiro não negativo?
- Em que ordem `soma_recursiva(3)` recebe os argumentos e devolve os resultados?
- Por que as duas implementações têm a mesma complexidade de tempo, mas custos de memória diferentes?
- Como provar que a função devolve `n × (n + 1) ÷ 2`?

## Exercícios

1. Implemente `fatorial(n)` com caso base e validação de entrada. Teste `0`, `1`, `5` e `-1`.
2. Implemente a versão iterativa de `fatorial` e compare o espaço usado pelas duas.
3. Escreva uma função recursiva que conte os itens de uma lista sem usar `len`. Qual é o custo de copiar fatias da lista em cada chamada?
4. Escreva um teste que detecte a ausência do caso base e explique a limitação de testar recursão infinita diretamente.

## Critério de conclusão

Você concluiu quando consegue desenhar as chamadas e os retornos de uma entrada nova, implementar um caso base correto, testar uma entrada inválida e justificar quando escolher recursão ou iteração.
