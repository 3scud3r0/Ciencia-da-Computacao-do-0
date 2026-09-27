# Capstone 01 — Mini computador

ISA de 16 bits + assembler de duas passagens + CPU virtual.

O assembler resolve labels e codifica instruções em words de 16 bits. A CPU possui 16 registradores, memória de words, program counter e ciclo fetch/decode/execute.

Instruções: `LOADI`, `LOAD`, `STORE`, `ADD`, `SUB`, `XOR`, `JNZ`, `JMP`, `OUT`, `HALT`.

Execute `python -m unittest -v`. Depois acrescente flags, CALL/RET, stack pointer e um formato de arquivo binário.
