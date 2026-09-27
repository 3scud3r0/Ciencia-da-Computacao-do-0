# Índice de conteúdo

Este arquivo separa **cobertura curricular**, **conteúdo autoral implementado** e **biblioteca de terceiros**.

## Cobertura curricular

- 19 módulos.
- 184 capítulos na super ementa.
- 56 projetos curriculares planejados.
- 10 capstones integradores dentro do módulo final.

A ementa completa está em [guia.md](./guia.md).

## Aulas autorais nucleares já escritas

1. [Como um computador executa um programa](./00-como-estudar/capitulos/01-como-um-computador-executa/)
2. [Bits, bytes e bases numéricas](./02-c-e-memoria/capitulos/01-bits-bytes-e-bases/)
3. [Ponteiros](./02-c-e-memoria/capitulos/04-ponteiros/)
4. [Hash tables](./04-estruturas-de-dados/capitulos/04-hash-tables/)
5. [Análise assintótica](./05-algoritmos/capitulos/01-analise-assintotica/)
6. [CPU: fetch/decode/execute](./06-arquitetura-de-computadores/capitulos/05-cpu/)
7. [Processos](./07-sistemas-operacionais/capitulos/02-processos/)
8. [TCP](./08-redes/capitulos/07-tcp/)
9. [HTTP](./08-redes/capitulos/09-http/)
10. [Índices de banco de dados](./09-bancos-de-dados/capitulos/05-indices/)
11. [Parsing](./10-linguagens-e-compiladores/capitulos/03-parsing/)
12. [Locks e atomics](./12-concorrencia-e-paralelismo/capitulos/03-locks-e-atomics/)
13. [Consenso distribuído](./13-sistemas-distribuidos/capitulos/08-consensus/)
14. [Hashes, MAC e KDF](./14-seguranca/capitulos/03-hashes-mac-kdf/)
15. [P, NP, NP-hard e NP-complete](./16-teoria-da-computacao/capitulos/08-classes-de-complexidade/)

## Projetos autorais executáveis

- [Vetor dinâmico em C](./02-c-e-memoria/projetos/vetor-dinamico/) — inclui sanitizers.
- [Hash table](./04-estruturas-de-dados/projetos/hashmap-python/).
- [Dijkstra](./05-algoritmos/projetos/roteador-dijkstra/).
- [ALU de 16 bits](./06-arquitetura-de-computadores/projetos/alu16/).
- [Servidor HTTP](./08-redes/projetos/http-server/).
- [Mini linguagem](./10-linguagens-e-compiladores/projetos/mini-lang/).
- [Thread pool](./12-concorrencia-e-paralelismo/projetos/thread-pool/).
- [Consistent hashing](./13-sistemas-distribuidos/projetos/consistent-hashing/).

Todos são executados pelo workflow de CI.

## Biblioteca de terceiros

### Snapshots principais

Em `vendor/`:

- TheAlgorithms/Python — MIT.
- TheAlgorithms/Algorithms-Explanation — MIT.
- Project Based Learning — MIT.
- ComputerScienceFromScratch — Apache-2.0.
- Coding Interview University — CC BY-SA 4.0.

### Comunidade auditada

`vendor/community/` contém atualmente **30 repositórios adicionais** com licença explicitamente identificada pelo GitHub e tamanho adequado ao repositório principal.

Exemplos: OS tutorial, DNS guide, compilers, C interpreter, hash table em C, Docker-like/container workshops, networking from scratch, JavaScript Algorithms, Tiny Raycaster, deep learning from scratch e outros.

A proveniência e o commit upstream estão em `vendor/community/manifest.json`.

### Catálogo externo

O site mantém **818 entradas** de Build Your Own X + Project Based Learning, inclusive links que não podem ser espelhados automaticamente.

## Estado real

A arquitetura, a ementa e a biblioteca externa são amplas. A autoria pedagógica completa dos 184 capítulos é um trabalho maior que os 15 capítulos nucleares já escritos; por isso este índice diferencia claramente o que já contém aula integral do que ainda está apenas representado na ementa.
