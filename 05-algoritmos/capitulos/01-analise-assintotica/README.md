# Análise assintótica

Complexidade descreve como o custo cresce quando a entrada cresce. Ela não mede segundos; mede uma função do tamanho da entrada.

Se `T(n) = 3n² + 10n + 500`, para crescimento assintótico o termo dominante é `n²`. Escrevemos `T(n) ∈ O(n²)`.

## Três símbolos

- **O(g(n))**: limite superior assintótico;
- **Ω(g(n))**: limite inferior;
- **Θ(g(n))**: limite superior e inferior da mesma ordem.

Dizer “é O(n²)” não significa necessariamente “é exatamente quadrático”. Uma função linear também pertence a O(n²). Quando queremos caracterizar precisamente a ordem, Θ é mais informativo.

## Exemplo

~~~python
for i in range(n):
    for j in range(n):
        work(i, j)
~~~

Há `n × n = n²` chamadas: Θ(n²).

Já:

~~~python
while n > 1:
    n //= 2
~~~

executa aproximadamente `log₂ n` iterações: Θ(log n).

## Custo amortizado

Uma operação isolada pode custar O(n), mas uma sequência ter média O(1). Vetores dinâmicos são o exemplo clássico: alguns `push` copiam todo o array, porém dobrar a capacidade faz o custo total de n inserções permanecer O(n).

## O objetivo real

Análise assintótica não substitui benchmark. Cache, constantes, alocação e paralelismo importam no mundo real. A análise responde primeiro: **o desenho escala?** O benchmark responde: **quanto custa nesta máquina e nesta carga?**
