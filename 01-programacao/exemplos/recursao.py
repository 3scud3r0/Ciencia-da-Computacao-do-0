"""Duas implementações equivalentes da soma de 1 até n."""


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
