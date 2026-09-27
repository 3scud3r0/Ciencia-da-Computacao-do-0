# Ciência da Computação do 0 — Guia Mestre

> Um currículo aberto, orientado a código e projetos, para aprender os fundamentos de uma graduação em Ciência da Computação sem reproduzir a duração de uma graduação tradicional.

## 1. Objetivo

Este repositório será uma formação autodidata completa em Ciência da Computação, com três compromissos:

1. **Explicar antes de abstrair** — cada conceito começa pela intuição e chega à formulação técnica.
2. **Implementar para compreender** — estruturas, algoritmos e sistemas importantes são construídos do zero sempre que isso for pedagogicamente útil.
3. **Conectar as camadas** — o aluno deve entender como uma linha de código chega a memória, CPU, sistema operacional, rede, banco de dados e sistemas distribuídos.

O currículo é deliberadamente mais condensado que uma graduação. Conteúdos redundantes, burocráticos ou já cobertos em uma formação típica de Ciência de Dados podem ser tratados como revisão ou trilha opcional.

## 2. Como cada capítulo será escrito

Cada capítulo seguirá, sempre que aplicável, esta estrutura:

- README.md — visão geral, objetivos e pré-requisitos.
- teoria.md — explicação intuitiva e formal.
- exemplos/ — exemplos mínimos executáveis.
- src/ — implementação principal.
- tests/ — testes automatizados.
- laboratorio.md — experimento guiado.
- exercicios.md — exercícios graduais.
- desafios.md — problemas de extensão.
- referencias.md — documentação, livros, papers e projetos upstream.
- quiz.md — verificação conceitual curta.
- projeto/ — quando o capítulo tiver um mini-projeto próprio.

Padrão pedagógico:

**conceito → motivação → modelo mental → formalização → código mínimo → implementação realista → testes → experimento → exercício → projeto → conexão com a próxima camada**

## 3. Linguagens do currículo

Não será usada uma única linguagem para tudo.

- **Python** — clareza, algoritmos, protótipos, redes, automação e conceitos de alto nível.
- **C** — memória, ponteiros, estruturas de dados, processos, sistemas e baixo nível.
- **Assembly x86-64** — chamadas, registradores, pilha, ABI e relação software/CPU.
- **SQL** — modelo relacional, índices, transações e consultas.
- **JavaScript/TypeScript** — navegador, HTTP, frontend e aplicações web.
- **Rust** — opcional avançado para ownership, segurança de memória e sistemas.
- **Bash** — ambiente Unix, pipelines e automação.

## 4. Convenções do repositório

Estrutura prevista:

~~~text
Ciencia-da-Computacao-do-0/
├── README.md
├── guia.md
├── LICENSE
├── CONTRIBUTING.md
├── CODE_OF_CONDUCT.md
├── .github/
│   ├── workflows/
│   └── ISSUE_TEMPLATE/
├── site/
├── assets/
├── ferramentas/
├── 00-como-estudar/
├── 01-programacao/
├── 02-c-e-memoria/
├── 03-matematica-discreta/
├── 04-estruturas-de-dados/
├── 05-algoritmos/
├── 06-arquitetura-de-computadores/
├── 07-sistemas-operacionais/
├── 08-redes/
├── 09-bancos-de-dados/
├── 10-linguagens-e-compiladores/
├── 11-engenharia-de-software/
├── 12-concorrencia-e-paralelismo/
├── 13-sistemas-distribuidos/
├── 14-seguranca/
├── 15-web-e-internet/
├── 16-teoria-da-computacao/
├── 17-topicos-avancados/
└── 18-projetos-finais/
~~~

## 5. Super ementa

### Módulo 00 — Ambiente, ferramentas e método

**Capítulo 00.1 — Como um computador executa um programa**
- código-fonte, compilador, interpretador, bytecode e máquina.
- processo, memória, CPU, sistema operacional e arquivos.
- laboratório: rastrear um programa Python do arquivo até o processo.

**Capítulo 00.2 — Terminal e shell**
- filesystem, caminhos, stdin/stdout/stderr, pipes, redirecionamento.
- comandos Unix essenciais.
- projeto: mini shell conceitual em Python.

**Capítulo 00.3 — Git e GitHub**
- objetos Git, commits, branches, merges, remotes.
- laboratório: inspecionar .git por baixo dos comandos.
- projeto: mini Git simplificado.

**Capítulo 00.4 — Ferramentas de desenvolvimento**
- editor, debugger, profiler, linter, formatter, build system e CI.
- projeto: pipeline de testes automático no GitHub Actions.

Arquivos principais:
~~~text
00-como-estudar/
├── 01-como-o-computador-executa/
├── 02-terminal-shell/
├── 03-git-github/
└── 04-ferramentas/
~~~

---

### Módulo 01 — Programação e pensamento computacional

**Capítulo 01.1 — Valores, tipos e representação**
- inteiros, ponto flutuante, booleanos, strings, mutabilidade.
- modelo de objetos em Python.

**Capítulo 01.2 — Controle de fluxo**
- condição, repetição, invariantes e estado.

**Capítulo 01.3 — Funções**
- stack frames, argumentos, escopo, closures e funções de ordem superior.

**Capítulo 01.4 — Recursão**
- pilha de chamadas, casos base, recursão estrutural.
- visualizador de stack.

**Capítulo 01.5 — Abstração e modularidade**
- módulos, interfaces, contratos e encapsulamento.

**Capítulo 01.6 — Orientação a objetos**
- composição, herança, polimorfismo, despacho dinâmico.

**Capítulo 01.7 — Programação funcional**
- funções puras, map/filter/reduce, imutabilidade.

**Capítulo 01.8 — Erros e depuração**
- exceções, assertions, debugger, tracing.

Projetos:
- interpretador de expressões aritméticas.
- sistema de arquivos virtual em memória.
- pequeno motor de regras.

---

### Módulo 02 — C, memória e representação de dados

**Capítulo 02.1 — Bits, bytes e bases numéricas**
- binário, hexadecimal, complemento de dois.
- operações bitwise.

**Capítulo 02.2 — Representação de inteiros e floats**
- overflow, IEEE 754, NaN, infinito, precisão.

**Capítulo 02.3 — C do zero**
- compilação, headers, linking, tipos, arrays e structs.

**Capítulo 02.4 — Ponteiros**
- endereço, dereference, pointer arithmetic e arrays.

**Capítulo 02.5 — Stack e heap**
- duração de objetos, frames, malloc/free.

**Capítulo 02.6 — Layout de memória**
- text, data, bss, heap, stack.
- inspeção com objdump/readelf/gdb.

**Capítulo 02.7 — Alocação dinâmica**
- malloc, fragmentation, free lists.
- projeto: mini malloc.

**Capítulo 02.8 — Segurança de memória**
- buffer overflow, use-after-free, double free.
- sanitizers.

Projetos:
- vetor dinâmico em C.
- string library.
- hashmap em C.
- mini alocador de memória.

---

### Módulo 03 — Matemática discreta para Ciência da Computação

**Capítulo 03.1 — Lógica proposicional**
- proposições, equivalência, implicação e tabelas-verdade.

**Capítulo 03.2 — Provas**
- prova direta, contraposição, contradição e indução.

**Capítulo 03.3 — Conjuntos e relações**
- operações, produto cartesiano, relações e equivalências.

**Capítulo 03.4 — Funções e cardinalidade**
- injetividade, sobrejetividade, bijeção e infinito contável.

**Capítulo 03.5 — Combinatória**
- princípio multiplicativo, permutações, combinações.

**Capítulo 03.6 — Probabilidade discreta**
- variáveis aleatórias, esperança e Bayes.

**Capítulo 03.7 — Grafos**
- caminhos, ciclos, conectividade, DAGs e árvores.

**Capítulo 03.8 — Recorrências**
- relações de recorrência e crescimento assintótico.

Projeto:
- biblioteca Python para experimentos de grafos, combinatória e probabilidade.

---

### Módulo 04 — Estruturas de dados

**Capítulo 04.1 — Arrays e arrays dinâmicos**
- locality, capacity, amortized analysis.

**Capítulo 04.2 — Linked lists**
- singly/doubly linked, sentinel nodes.

**Capítulo 04.3 — Stack, queue e deque**

**Capítulo 04.4 — Hash tables**
- hashing, colisões, chaining, open addressing.

**Capítulo 04.5 — Árvores**
- binary trees, traversal e propriedades.

**Capítulo 04.6 — Binary Search Trees**

**Capítulo 04.7 — Árvores balanceadas**
- AVL e Red-Black Tree.

**Capítulo 04.8 — Heaps e priority queues**

**Capítulo 04.9 — Tries**

**Capítulo 04.10 — Union-Find / Disjoint Set**

**Capítulo 04.11 — Grafos**
- adjacency list/matrix.

**Capítulo 04.12 — B-Trees e B+ Trees**
- ponte para bancos de dados e filesystems.

Projetos:
- biblioteca de estruturas em Python.
- segunda implementação crítica em C.
- visualizador interativo de árvores.
- índice B+ Tree persistente.

---

### Módulo 05 — Algoritmos

**Capítulo 05.1 — Análise assintótica**
- O, Ω, Θ, best/average/worst case.

**Capítulo 05.2 — Busca**
- linear, binary search e variantes.

**Capítulo 05.3 — Ordenação**
- insertion, selection, merge, quick, heap, counting e radix.

**Capítulo 05.4 — Divide and conquer**

**Capítulo 05.5 — Greedy algorithms**

**Capítulo 05.6 — Programação dinâmica**
- memoization, tabulation, estados e transições.

**Capítulo 05.7 — Backtracking e branch-and-bound**

**Capítulo 05.8 — Algoritmos em grafos I**
- BFS, DFS, topological sort.

**Capítulo 05.9 — Algoritmos em grafos II**
- Dijkstra, Bellman-Ford, Floyd-Warshall.

**Capítulo 05.10 — Minimum Spanning Tree**
- Kruskal e Prim.

**Capítulo 05.11 — String algorithms**
- KMP, Rabin-Karp, suffix concepts.

**Capítulo 05.12 — Randomized algorithms**

Projetos:
- biblioteca de algoritmos com benchmarks.
- visualizador de algoritmos.
- roteador de caminhos mínimos.
- resolvedor de problemas por programação dinâmica.

---

### Módulo 06 — Arquitetura de computadores

**Capítulo 06.1 — Portas lógicas**
- AND, OR, NOT, XOR e circuitos combinacionais.

**Capítulo 06.2 — Álgebra booleana e circuitos**

**Capítulo 06.3 — Somadores e ALU**

**Capítulo 06.4 — Clock, registradores e memória**

**Capítulo 06.5 — CPU**
- fetch/decode/execute.

**Capítulo 06.6 — ISA**
- instruções, addressing modes e machine code.

**Capítulo 06.7 — Assembly x86-64**
- registradores, stack, CALL/RET, ABI.

**Capítulo 06.8 — Cache**
- locality, cache lines, associativity e misses.

**Capítulo 06.9 — Pipeline**
- hazards, branch prediction e superscalaridade.

**Capítulo 06.10 — Virtualização do hardware**

Projetos:
- simulador de portas lógicas.
- ALU.
- CPU educacional de 16 bits.
- assembler.
- emulador CHIP-8.
- analisador simples de cache.

---

### Módulo 07 — Sistemas Operacionais

**Capítulo 07.1 — Kernel e user space**

**Capítulo 07.2 — Processos**
- fork/exec/wait, PCB, context switch.

**Capítulo 07.3 — Threads**
- criação, compartilhamento e sincronização.

**Capítulo 07.4 — Scheduling**
- FCFS, SJF, Round Robin, priorities.

**Capítulo 07.5 — Concorrência**
- race conditions, mutex, semaphore e monitor.

**Capítulo 07.6 — Deadlocks**
- condições de Coffman e estratégias.

**Capítulo 07.7 — Memória virtual**
- pages, page tables, TLB e page faults.

**Capítulo 07.8 — Alocação de memória**

**Capítulo 07.9 — Filesystems**
- inode, directory, block allocation, journaling.

**Capítulo 07.10 — I/O**
- interrupts, DMA e drivers.

**Capítulo 07.11 — Syscalls**

**Capítulo 07.12 — Containers**
- namespaces, cgroups e isolamento.

Projetos:
- scheduler simulator.
- mini shell POSIX em C.
- memory allocator.
- filesystem educacional.
- container minimalista em Linux.

---

### Módulo 08 — Redes de computadores

**Capítulo 08.1 — Modelo em camadas**
- TCP/IP e comparação com OSI.

**Capítulo 08.2 — Ethernet**
- frames, MAC e switches.

**Capítulo 08.3 — IP**
- IPv4/IPv6, CIDR, routing e TTL.

**Capítulo 08.4 — ARP/Neighbor Discovery**

**Capítulo 08.5 — ICMP**

**Capítulo 08.6 — UDP**

**Capítulo 08.7 — TCP**
- handshake, sequence numbers, reliability, congestion.

**Capítulo 08.8 — DNS**
- resolução recursiva, registros e caching.

**Capítulo 08.9 — HTTP**
- HTTP/1.1, HTTP/2 e conceitos de HTTP/3.

**Capítulo 08.10 — TLS**
- handshake, certificados e criptografia aplicada.

**Capítulo 08.11 — Sockets**
- cliente/servidor.

**Capítulo 08.12 — NAT, firewalls e proxies**

Projetos:
- packet parser.
- cliente DNS.
- servidor HTTP do zero.
- proxy HTTP.
- chat TCP.
- traceroute simplificado.

---

### Módulo 09 — Bancos de dados

**Capítulo 09.1 — Modelo relacional**

**Capítulo 09.2 — SQL**
- SELECT, JOIN, agregações, subqueries e CTE.

**Capítulo 09.3 — Álgebra relacional**

**Capítulo 09.4 — Armazenamento em páginas**

**Capítulo 09.5 — Índices**
- B+ Tree, hash index.

**Capítulo 09.6 — Query processing**

**Capítulo 09.7 — Query optimizer**
- planos, custos e estatísticas.

**Capítulo 09.8 — Transações**
- ACID.

**Capítulo 09.9 — Concorrência**
- locks, MVCC, isolation levels.

**Capítulo 09.10 — Recovery**
- WAL, checkpoints.

**Capítulo 09.11 — NoSQL**
- key-value, document, columnar e graph.

Projetos:
- parser SQL simples.
- storage engine.
- B+ Tree persistente.
- mini banco relacional.
- transações com WAL.

---

### Módulo 10 — Linguagens de programação e compiladores

**Capítulo 10.1 — Como linguagens são implementadas**

**Capítulo 10.2 — Léxico e tokens**

**Capítulo 10.3 — Parsing**
- recursive descent, precedence e grammar.

**Capítulo 10.4 — AST**

**Capítulo 10.5 — Interpretadores**

**Capítulo 10.6 — Escopo e ambientes**

**Capítulo 10.7 — Tipos**
- static/dynamic, inference e checking.

**Capítulo 10.8 — Bytecode e máquinas virtuais**

**Capítulo 10.9 — Compilação**
- IR e code generation.

**Capítulo 10.10 — Garbage collection**
- reference counting, mark-and-sweep, generational concepts.

**Capítulo 10.11 — Otimizações**

Projetos:
- calculadora com parser.
- linguagem interpretada própria.
- bytecode VM.
- garbage collector didático.
- pequeno compilador.

---

### Módulo 11 — Engenharia de software

**Capítulo 11.1 — APIs e contratos**

**Capítulo 11.2 — Testes**
- unit, integration, property-based e end-to-end.

**Capítulo 11.3 — Design e modularidade**
- coupling, cohesion, SOLID onde fizer sentido.

**Capítulo 11.4 — Refatoração**

**Capítulo 11.5 — Observabilidade**
- logs, metrics e traces.

**Capítulo 11.6 — Performance**
- profiling, benchmarking, latency e throughput.

**Capítulo 11.7 — Build e dependências**

**Capítulo 11.8 — CI/CD**

**Capítulo 11.9 — Versionamento e releases**

**Capítulo 11.10 — Arquitetura de software**
- monolith, modular monolith, services e event-driven.

Projeto:
- serviço completo testado, instrumentado e automatizado.

---

### Módulo 12 — Concorrência e paralelismo

**Capítulo 12.1 — Concorrência vs paralelismo**

**Capítulo 12.2 — Threads e memória compartilhada**

**Capítulo 12.3 — Locks e atomics**

**Capítulo 12.4 — Semáforos e condition variables**

**Capítulo 12.5 — Lock-free: fundamentos**

**Capítulo 12.6 — Futures, async/await e event loops**

**Capítulo 12.7 — Multiprocessing**

**Capítulo 12.8 — SIMD e paralelismo de dados**

Projetos:
- thread pool.
- producer/consumer.
- servidor concorrente.
- event loop minimalista.

---

### Módulo 13 — Sistemas distribuídos

**Capítulo 13.1 — Por que sistemas distribuídos são difíceis**

**Capítulo 13.2 — Tempo e relógios**
- wall clock, monotonic, Lamport e vector clocks.

**Capítulo 13.3 — Falhas**
- crash, omission, partition e Byzantine overview.

**Capítulo 13.4 — Replicação**

**Capítulo 13.5 — Particionamento e sharding**

**Capítulo 13.6 — Consistência**
- linearizability, eventual consistency.

**Capítulo 13.7 — CAP e PACELC**

**Capítulo 13.8 — Consensus**
- conceitos de Paxos e implementação educacional de Raft.

**Capítulo 13.9 — Leader election**

**Capítulo 13.10 — Distributed transactions**
- 2PC e sagas.

**Capítulo 13.11 — Message queues e logs distribuídos**

**Capítulo 13.12 — MapReduce**

Projetos:
- key-value store replicado.
- consistent hashing.
- Raft educacional.
- fila distribuída simples.
- mini MapReduce.

---

### Módulo 14 — Segurança de computadores

**Capítulo 14.1 — Threat modeling**

**Capítulo 14.2 — Criptografia simétrica**

**Capítulo 14.3 — Hashes, MAC e KDF**

**Capítulo 14.4 — Criptografia assimétrica**

**Capítulo 14.5 — Assinaturas digitais**

**Capítulo 14.6 — PKI e certificados**

**Capítulo 14.7 — TLS por dentro**

**Capítulo 14.8 — Autenticação e autorização**

**Capítulo 14.9 — Segurança web**
- XSS, CSRF, SQL injection, SSRF, session security.

**Capítulo 14.10 — Segurança de memória**

**Capítulo 14.11 — Sandboxing e privilege separation**

**Capítulo 14.12 — Secure coding**

Projetos:
- password vault educacional.
- sistema de autenticação seguro.
- laboratório local de vulnerabilidades deliberadas.
- implementação didática de primitives apenas para aprendizagem, nunca para produção.

---

### Módulo 15 — Web e Internet

**Capítulo 15.1 — Do URL ao pixel**
- DNS → TCP/QUIC → TLS → HTTP → HTML/CSS/JS → render.

**Capítulo 15.2 — HTML e DOM**

**Capítulo 15.3 — CSS e layout**

**Capítulo 15.4 — JavaScript no navegador**
- event loop, promises e Web APIs.

**Capítulo 15.5 — Backend**
- routing, middleware, sessions e APIs.

**Capítulo 15.6 — REST, RPC e GraphQL**

**Capítulo 15.7 — Caching e CDNs**

**Capítulo 15.8 — WebSockets e realtime**

Projeto:
- aplicação web full-stack construída após implementar um servidor HTTP simples.

---

### Módulo 16 — Teoria da computação

**Capítulo 16.1 — Linguagens formais**

**Capítulo 16.2 — Autômatos finitos**

**Capítulo 16.3 — Expressões regulares**

**Capítulo 16.4 — Gramáticas livres de contexto**

**Capítulo 16.5 — Máquinas de Turing**

**Capítulo 16.6 — Computabilidade**

**Capítulo 16.7 — Problema da parada**

**Capítulo 16.8 — Classes de complexidade**
- P, NP, NP-hard e NP-complete.

**Capítulo 16.9 — Reduções**

Projetos:
- simulador de DFA/NFA.
- conversor regex → autômato simplificado.
- máquina de Turing educacional.
- explorador interativo de reduções.

---

### Módulo 17 — Tópicos avançados

Trilhas opcionais.

**17A — Computação gráfica**
- vetores, matrizes, transforms, rasterization, shaders, ray tracing.
- projeto: renderer 3D/ray tracer.

**17B — Inteligência artificial**
- search, CSPs, minimax, representação de conhecimento.
- projeto: agente de jogos.

**17C — Machine Learning por dentro**
- regressão, otimização, backpropagation e redes neurais implementadas do zero.
- destinada a compreender mecanismos, não repetir uma formação de Data Science.

**17D — Sistemas de arquivos e storage avançado**
- LSM trees, compression, bloom filters.

**17E — Cloud e infraestrutura**
- containers, orchestration, load balancing, service discovery.

**17F — Rust para sistemas**
- ownership, borrowing, lifetimes e concurrency safety.

**17G — Information retrieval**
- inverted indexes, ranking e search engines.

**17H — Compressão**
- Huffman, LZ family e conceitos de entropy coding.

---

### Módulo 18 — Projetos finais integradores

Os projetos finais devem unir várias camadas do currículo.

**Projeto final A — Mini computador**
- portas lógicas.
- ALU.
- CPU.
- assembler.
- VM.
- linguagem simples.

**Projeto final B — Mini sistema operacional**
- boot conceitual.
- processos/threads.
- scheduler.
- memória.
- filesystem mínimo.

**Projeto final C — Banco de dados**
- pages.
- B+ Tree.
- SQL subset.
- planner.
- transactions.
- WAL.

**Projeto final D — Redis-like**
- protocolo.
- event loop.
- estruturas.
- persistência.
- replicação opcional.

**Projeto final E — Git-like**
- blobs.
- trees.
- commits.
- branches.
- diff simplificado.

**Projeto final F — Container runtime mínimo**
- namespaces.
- cgroups.
- filesystem isolation.
- processo init.

**Projeto final G — Linguagem de programação**
- lexer.
- parser.
- AST.
- interpreter/VM.
- GC opcional.

**Projeto final H — Distributed key-value store**
- networking.
- replication.
- consistent hashing.
- consensus opcional.
- persistence.

**Projeto final I — Search engine**
- crawler local/controlado.
- tokenizer.
- inverted index.
- ranking.
- web UI.

**Projeto final J — Stack web completo**
- HTTP server.
- API.
- database.
- authentication.
- caching.
- frontend.
- observability.

## 6. Catálogo de implementações que o repositório deverá conter

### Estruturas de dados
- DynamicArray
- LinkedList
- DoublyLinkedList
- Stack
- Queue
- Deque
- HashMap
- HashSet
- BinaryTree
- BST
- AVL
- RedBlackTree
- BinaryHeap
- Trie
- UnionFind
- Graph
- BTree
- BPlusTree
- BloomFilter

### Algoritmos
- binary search
- merge sort
- quicksort
- heapsort
- counting/radix sort
- BFS/DFS
- topological sort
- Dijkstra
- Bellman-Ford
- Floyd-Warshall
- Prim
- Kruskal
- KMP
- Rabin-Karp
- dynamic programming patterns
- greedy patterns
- backtracking
- randomized algorithms

### Sistemas
- dynamic array em C
- hashmap em C
- malloc educacional
- shell
- scheduler simulator
- page replacement simulator
- filesystem
- thread pool
- event loop
- HTTP server
- DNS client
- TCP chat
- proxy
- cache simulator
- assembler
- CHIP-8 emulator
- VM
- garbage collector
- compiler/interpreter

### Dados e sistemas distribuídos
- B+ Tree
- storage engine
- SQL parser
- WAL
- transaction manager
- key-value store
- consistent hashing
- replication
- Raft educacional
- message queue
- MapReduce

## 7. Relação com projetos públicos

Este projeto poderá estudar e referenciar iniciativas como:

- OSSU Computer Science
- Teach Yourself CS
- CS Primer
- The Algorithms
- Build Your Own X
- Project Based Learning
- Coding Interview University
- Nand2Tetris
- OSTEP
- Crafting Interpreters
- xv6

A regra editorial será: **preferir implementações próprias e didáticas**. Quando código de terceiros for incorporado de forma compatível com sua licença, a atribuição, aviso de copyright e licença correspondentes deverão ser preservados. O objetivo não é copiar repositórios inteiros, mas transformar ideias importantes em uma narrativa curricular coerente.

## 8. Níveis de profundidade

Cada capítulo terá marcadores:

- **Essencial** — necessário para a trilha principal.
- **Profundo** — implementação que revela o funcionamento interno.
- **Avançado** — extensão para quem quer nível de systems/programming research.
- **Opcional** — útil, mas não bloqueia os capítulos seguintes.

Isso permite seguir duas velocidades:

### Trilha acelerada
Foco em Essencial + projetos-chave.

Estimativa-alvo editorial: cerca de **150–220 horas**, dependendo do conhecimento prévio e do número de exercícios executados.

### Trilha completa
Essencial + Profundo + Avançado + projetos finais.

Carga aberta: pode ultrapassar **400 horas**, especialmente se todos os projetos forem implementados integralmente.

## 9. Sequência acelerada recomendada

Para quem já estudou Ciência de Dados:

~~~text
Ferramentas
   ↓
C + memória
   ↓
Matemática discreta essencial
   ↓
Estruturas de dados
   ↓
Algoritmos
   ↓
Assembly + arquitetura
   ↓
Sistemas operacionais
   ↓
Redes
   ↓
Bancos de dados por dentro
   ↓
Compiladores e linguagens
   ↓
Concorrência
   ↓
Sistemas distribuídos
   ↓
Segurança
   ↓
Teoria da computação
   ↓
Projeto final
~~~

Programação Python básica, estatística e Machine Learning podem ser tratados como revisão quando já dominados.

## 10. Padrão de qualidade de código

Toda implementação importante deverá possuir:

- execução reproduzível;
- comentários focados no motivo, não na tradução óbvia da linha;
- testes automatizados;
- casos extremos;
- análise de complexidade;
- explicação de memória quando relevante;
- benchmark quando relevante;
- versão mínima pedagógica;
- versão mais realista quando a diferença for instrutiva;
- indicação explícita do que foi simplificado;
- referências técnicas.

## 11. Padrão de explicação

Uma explicação não será considerada concluída apenas porque apresentou código.

Exemplo de requisito para uma hash table:

1. por que arrays não resolvem todas as buscas;
2. definição de função hash;
3. colisões;
4. chaining;
5. open addressing;
6. load factor;
7. resizing;
8. complexidade média e pior caso;
9. implementação em Python;
10. implementação crítica em C;
11. testes;
12. benchmark;
13. relação com dictionaries reais;
14. exercícios de modificação.

## 12. GitHub Pages

O site deverá ser uma camada de navegação sobre o conteúdo do repositório, não uma cópia separada.

Recursos planejados:

- página inicial com mapa curricular;
- progresso por módulo;
- busca;
- navegação anterior/próximo;
- filtro Essencial / Profundo / Avançado;
- blocos de código com syntax highlighting;
- diagramas;
- quizzes;
- caixas “o que está acontecendo no computador?”;
- links diretos para arquivos e testes;
- páginas de projeto;
- modo claro/escuro;
- layout responsivo;
- índice global de conceitos;
- glossário;
- dependências entre capítulos;
- trilha acelerada para quem já vem de Data Science.

## 13. Três direções visuais candidatas

### Visual A — “Laboratório / Terminal”
Identidade escura, técnica, inspirada em terminal moderno e ferramentas de sistemas. Sidebar fixa com módulos, mapa de progresso, código em primeiro plano e diagramas minimalistas.

Ideal para transmitir: **engenharia, baixo nível, precisão e construção**.

### Visual B — “Livro Universitário Moderno”
Fundo claro, tipografia editorial, largura confortável de leitura, notas laterais, diagramas amplos e código integrado ao texto.

Ideal para transmitir: **curso sério, legibilidade e estudo prolongado**.

### Visual C — “Mapa Interativo da Computação”
Home centrada em um grafo/roadmap visual. Cada módulo é um nó e as dependências aparecem como conexões. Dentro dos capítulos, o layout volta a ser documental com cards de laboratório e projetos.

Ideal para transmitir: **exploração, progressão e visão sistêmica de toda a Ciência da Computação**.

## 14. Marcos de implementação

### Marco 1 — Fundação
- guia.md
- README.md
- arquitetura do site
- identidade visual escolhida
- GitHub Pages
- sistema de navegação

### Marco 2 — Fundamentos
- módulos 00–05
- testes e exercícios
- primeiro conjunto de visualizações

### Marco 3 — Sistemas
- módulos 06–10
- emulador, shell, HTTP server, banco e interpretador

### Marco 4 — Engenharia avançada
- módulos 11–16
- concorrência, distribuídos, segurança e teoria

### Marco 5 — Trilhas e capstones
- módulo 17
- módulo 18
- projetos finais integradores

## 15. Definição de “concluído”

A pessoa que concluir a trilha principal deverá ser capaz de explicar, sem tratar as camadas como caixas mágicas:

- como dados são representados em bits;
- como memória e ponteiros funcionam;
- como estruturas de dados são construídas;
- como algoritmos são analisados;
- como código chega a instruções da CPU;
- como processos, threads e memória virtual funcionam;
- como uma mensagem atravessa uma rede;
- como um banco encontra e protege dados;
- como uma linguagem é interpretada ou compilada;
- como concorrência produz race conditions;
- como serviços distribuídos lidam com falhas;
- como mecanismos básicos de segurança funcionam;
- quais problemas são computáveis e quais limites teóricos existem;
- como integrar essas camadas em software real.

---

Este arquivo é a **especificação curricular principal** do projeto. A implementação de capítulos poderá evoluir, mas novas adições devem preservar a sequência conceitual e evitar transformar o repositório em uma coleção desconexa de links.
