# ALU de 16 bits

A ALU recebe operandos, uma operação e devolve resultado + flags.

`u16(x) = x & 0xFFFF` modela o descarte dos bits que não cabem no datapath de 16 bits. As flags são Z (zero), N (negativo), C (carry) e V (overflow com sinal).

Explique a diferença entre `0xFFFF + 1` e `32767 + 1`: ambos transbordam algum limite, mas **carry** e **signed overflow** expressam propriedades diferentes.
