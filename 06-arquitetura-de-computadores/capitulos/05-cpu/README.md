# CPU: fetch, decode, execute

Uma CPU pode ser entendida inicialmente como um estado composto de registradores, contador de programa e memória, mais um circuito que transforma esse estado a cada ciclo.

~~~text
PC → memória de instruções → instrução
                           ↓ decode
registradores → ALU → resultado
      ↑             ↓
      └──── write-back
~~~

## Fetch

O **program counter (PC)** indica onde está a próxima instrução. O processador busca os bytes naquele endereço.

## Decode

Bits da instrução especificam operação e operandos: por exemplo, “some os registradores A e B e escreva em C”. O formato exato é definido pela ISA.

## Execute

A ALU realiza aritmética/lógica; instruções de load/store acessam memória; branches podem alterar o PC.

## Registradores vs RAM

Registradores são poucos e diretamente integrados ao datapath da CPU. RAM é muito maior e mais lenta. Caches ocupam o meio: pequenas memórias que exploram localidade temporal e espacial.

## Uma abstração importante

O código:

~~~c
c = a + b;
~~~

não determina uma única sequência universal de instruções. O compilador escolhe instruções segundo arquitetura, otimização e contexto. O resultado observável pode ser igual mesmo com assembly diferente.

Abra `projetos/alu16`: as operações ADD, SUB, AND, OR e flags representam uma peça do datapath. A etapa seguinte é combinar ALU, registradores e PC em uma CPU educacional.
