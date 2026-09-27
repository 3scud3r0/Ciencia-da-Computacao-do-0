#!/usr/bin/env python3
from __future__ import annotations
import json,re,unicodedata
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
CURRICULUM=ROOT/"site"/"data"/"curriculum.json"
LESSONS_JSON=ROOT/"site"/"data"/"lessons.json"
REPO_BASE="https://github.com/3scud3r0/Ciencia-da-Computacao-do-0/tree/main/"

MODULE_OVERVIEW={
"00":"Ferramentas não são acessórios: shell, Git, debugger, profiler, build e CI formam a camada operacional usada para investigar, reproduzir e automatizar o comportamento do software.",
"01":"Programar é modelar estado e transformação. Este módulo conecta sintaxe a conceitos duráveis: valores, controle, funções, abstração, composição e tratamento explícito de falhas.",
"02":"C remove boa parte das proteções de linguagens de alto nível. Isso torna visíveis representação binária, endereços, lifetime, layout e responsabilidade pela memória.",
"03":"Matemática discreta fornece a linguagem para especificar e provar propriedades de programas: lógica, conjuntos, contagem, probabilidade, grafos e recorrências.",
"04":"Uma estrutura de dados é uma escolha de representação que torna algumas operações baratas e outras caras. O foco é manter invariantes e entender custo temporal e espacial.",
"05":"Algoritmos transformam problemas em procedimentos e permitem comparar soluções independentemente de uma máquina específica. Correção e complexidade caminham juntas.",
"06":"Arquitetura explica como instruções, registradores, ALU, memória e caches transformam software em mudanças físicas de estado numa máquina.",
"07":"O sistema operacional virtualiza e arbitra recursos: CPU, memória, armazenamento e dispositivos. Processos, threads e syscalls são a interface entre programas e kernel.",
"08":"Redes são camadas de protocolos com responsabilidades separadas. O objetivo é seguir bytes desde um processo, pela pilha, pela rede, até outro processo.",
"09":"Bancos de dados combinam estruturas, armazenamento, concorrência e recuperação. O desafio é responder consultas corretamente e sobreviver a falhas com desempenho aceitável.",
"10":"Linguagens de programação são sistemas implementados. Lexer, parser, AST, tipos, VM, compilador e GC são mecanismos concretos que podem ser construídos.",
"11":"Engenharia de software trata de manter sistemas compreensíveis e verificáveis ao longo do tempo: contratos, testes, observabilidade, performance, automação e arquitetura.",
"12":"Concorrência introduz múltiplas linhas de execução e, com elas, interleavings. O foco é preservar invariantes usando sincronização e escolher o modelo adequado ao problema.",
"13":"Sistemas distribuídos adicionam redes e falhas independentes. Sem relógio global e sem entrega garantida, consistência, replicação e consenso exigem protocolos explícitos.",
"14":"Segurança começa por propriedades e ameaças, não por ferramentas. Criptografia, autenticação, isolamento e programação segura protegem fronteiras diferentes.",
"15":"A Web conecta navegador, protocolos, servidor e armazenamento. Entender o caminho URL→rede→backend→DOM evita tratar frameworks como caixas mágicas.",
"16":"Teoria da computação formaliza linguagens, máquinas e limites. Alguns problemas são fáceis, outros caros, e alguns sequer podem ser decididos por algoritmo geral.",
"17":"Tópicos avançados reutilizam fundamentos em domínios diferentes: álgebra em gráficos, otimização em ML, estruturas em storage e índices em mecanismos de busca.",
"18":"Capstones integram várias camadas. A meta é construir sistemas que possuam especificação, testes, persistência, protocolo, observabilidade e documentação suficientes para serem explicados de ponta a ponta."
}

LABS={
"00":"projetos/mini-git","01":"projetos/virtual-fs","02":"projetos/vetor-dinamico","03":"projetos/discrete-lab",
"04":"projetos/hashmap-python","05":"projetos/roteador-dijkstra","06":"projetos/alu16","07":"projetos/scheduler-simulator",
"08":"projetos/http-server","09":"projetos/bplus-tree","10":"projetos/mini-lang","11":"projetos/instrumented-service",
"12":"projetos/thread-pool","13":"projetos/consistent-hashing","14":"projetos/password-store","15":"projetos/mini-web",
"16":"projetos/automatos","17":"projetos/raytracer","18":"projetos/kv-http-service"
}

FOCUS={
"00":[
"Executar um programa envolve resolver o executável, criar um processo, mapear memória, carregar dependências e transferir controle para instruções que a CPU entende. Programa armazenado e processo em execução são distintos.",
"O shell faz parsing de comandos, expansão, redirecionamento e pipelines. stdin, stdout e stderr são streams; pipes conectam saída de um processo à entrada de outro.",
"Git é um banco content-addressed de objetos: blobs, trees e commits. Branches são referências móveis; commits formam um DAG e merges combinam históricos.",
"Debugger controla execução e inspeciona estado; profiler mede onde tempo ou memória são gastos; build transforma fontes em artefatos; CI torna as verificações reproduzíveis."
],
"01":[
"Valores possuem representação e operações válidas; tipos restringem interpretações. Mutabilidade define se identidade e estado podem mudar depois da criação.",
"Condicionais escolhem caminhos e loops repetem transições de estado. Invariantes são propriedades que devem permanecer verdadeiras antes e depois de cada iteração.",
"Função cria uma fronteira de abstração. Escopo define visibilidade; closures capturam ambiente léxico; argumentos e retornos formam contratos.",
"Recursão reduz um problema a instâncias menores até um caso base. Cada chamada mantém estado lógico; ausência de progresso produz recursão infinita.",
"Modularidade reduz acoplamento ao esconder decisões internas atrás de interfaces. Uma abstração boa expõe o necessário e preserva invariantes.",
"Orientação a objetos combina estado e comportamento. Composição modela relações tem-um; herança exige substituição sem quebrar expectativas do tipo base.",
"Programação funcional enfatiza funções puras, composição e imutabilidade. map/filter/reduce são padrões de transformação e pureza facilita raciocínio e testes.",
"Erros devem produzir evidência. Exceções propagam falhas, assertions verificam invariantes e debugger/tracing ajudam a localizar a primeira divergência."
],
"02":[
"Bits não carregam significado por si; convenções decidem se um padrão representa unsigned, signed, caractere ou flags. Hexadecimal agrupa quatro bits e torna máscaras legíveis.",
"Inteiros de largura fixa sofrem overflow modular; IEEE 754 separa sinal, expoente e fração. Nem todo decimal tem representação binária finita.",
"C compila tipos e operações para instruções próximas do hardware. Headers declaram interfaces, objetos compilados contêm código/dados e linker resolve símbolos.",
"Ponteiro armazena endereço e seu tipo orienta a interpretação dos bytes. Pointer arithmetic avança em elementos e dereference só é válido durante o lifetime do objeto.",
"Stack normalmente contém frames e objetos automáticos; heap contém alocações de duração explícita. Lifetime e região de armazenamento não são o mesmo conceito.",
"Um processo separa código, dados, BSS, heap, mappings e stack em espaço virtual. readelf, objdump e gdb permitem observar esse layout.",
"Alocador precisa localizar blocos, dividir, reutilizar e coalescer espaço. Fragmentação interna desperdiça dentro de blocos; externa cria espaços livres pouco utilizáveis.",
"Buffer overflow, use-after-free, double free e integer overflow podem corromper estado. Sanitizers instrumentam execução e tornam vários bugs reproduzíveis."
],
"03":[
"Lógica proposicional combina verdade com ¬, ∧, ∨ e →. Equivalências permitem reescrever expressões e tabelas-verdade verificam fórmulas finitas.",
"Provas estabelecem propriedades para toda uma classe. Direta, contraposição, contradição e indução são técnicas diferentes para ligar hipóteses a conclusões.",
"Conjuntos modelam coleções e relações são subconjuntos de produtos cartesianos. Reflexividade, simetria e transitividade definem equivalência; ordens parciais modelam precedência.",
"Funções podem ser injetivas, sobrejetivas ou bijetivas. Bijeções comparam cardinalidades e diagonalização mostra que alguns infinitos não são enumeráveis.",
"Princípios aditivo e multiplicativo contam escolhas. Permutações consideram ordem; combinações não: C(n,k)=n!/(k!(n-k)!).",
"Probabilidade discreta atribui massa a resultados. Esperança é linear mesmo sem independência; Bayes inverte condicionais por P(A|B)=P(B|A)P(A)/P(B).",
"Grafos modelam vértices e arestas. Caminhos, ciclos, conectividade, árvores e DAGs representam redes, dependências e espaços de estado.",
"Recorrências descrevem custo em termos de instâncias menores. Expandir, substituir ou usar árvore de recursão revela crescimento como Θ(n log n)."
],
"04":[
"Array é memória contígua com acesso indexado O(1). Inserir no meio desloca O(n); arrays dinâmicos reservam capacidade para append amortizado O(1).",
"Linked list guarda elementos em nós ligados. Inserção local é O(1) quando o nó é conhecido, mas busca por índice é O(n) e localidade de cache é pior.",
"Stack implementa LIFO, queue FIFO e deque opera nas duas extremidades. Essas interfaces aparecem em parsers, BFS, schedulers e buffers.",
"Hash table transforma chave em índice e precisa tratar colisões. Chaining/open addressing e load factor determinam custo esperado e momento de resize.",
"Árvore organiza nós hierarquicamente. Profundidade, altura e subárvore permitem raciocínio recursivo; traversals diferem pela ordem da visita.",
"BST mantém esquerda < nó < direita. Busca custa O(h); árvore degenerada tem h=n e uma balanceada mantém h=Θ(log n).",
"AVL usa fator de balanceamento; Red-Black usa regras de cores. Ambas preservam operações O(log n) por rotações e manutenção de invariantes.",
"Heap mantém ordem parcial. O mínimo/máximo fica na raiz e push/pop custam O(log n), sustentando priority queues e heapsort.",
"Trie percorre símbolos da chave por arestas. Busca custa O(L) e prefixos compartilhados reutilizam a mesma trajetória.",
"Union-Find mantém conjuntos disjuntos. Union by rank/size e path compression dão custo amortizado O(α(n)), quase constante.",
"Grafos usam matriz O(V²) ou listas O(V+E). A escolha altera custo de consultar arestas, iterar vizinhos e memória.",
"B-Tree/B+ Tree usam muitos filhos para reduzir altura e page I/O. B+ Trees concentram registros nas folhas ligadas e favorecem range scans."
],
"05":[
"Análise assintótica compara crescimento: O limita acima, Ω abaixo e Θ caracteriza ordem. Benchmarks entram depois para custos concretos.",
"Busca linear custa Θ(n); binary search custa Θ(log n), mas exige ordem e acesso eficiente ao elemento central.",
"Insertion sort é bom para pequenos/quase ordenados; merge sort garante Θ(n log n); quicksort é ótimo em média; heapsort garante Θ(n log n).",
"Divide and conquer divide, resolve subproblemas e combina. Recorrências como T(n)=aT(n/b)+f(n) ajudam a derivar custo.",
"Greedy escolhe decisão local sem revisitar. Correção exige propriedade de escolha gulosa e subestrutura ótima, não apenas intuição.",
"Programação dinâmica reutiliza subproblemas sobrepostos. Memoization é top-down, tabulation bottom-up; o trabalho principal é definir estado/transição.",
"Backtracking percorre árvore de decisões e desfaz escolhas inviáveis. Pruning e branch-and-bound reduzem o espaço explorado.",
"BFS usa queue e descobre por distância em arestas; DFS usa stack/recursão. Com listas, ambos custam O(V+E).",
"Dijkstra exige pesos não negativos; Bellman-Ford tolera negativos e detecta ciclos negativos; Floyd-Warshall resolve all-pairs em O(V³).",
"MST conecta todos os vértices com peso total mínimo. Kruskal ordena arestas e usa Union-Find; Prim cresce pela fronteira.",
"KMP evita retroceder no texto usando informação de prefixos; Rabin-Karp usa hashes e precisa confirmar colisões.",
"Randomizados usam aleatoriedade para tempo esperado ou garantia probabilística. Diferencie Monte Carlo de Las Vegas e controle seeds em testes."
],
"06":[
"Portas AND/OR/NOT/XOR implementam funções booleanas; circuitos combinacionais dependem apenas das entradas atuais. Multiplexadores escolhem fontes.",
"Álgebra booleana simplifica circuitos via identidades e De Morgan. Menos portas pode reduzir atraso, área e consumo.",
"Somadores propagam carry; ALU seleciona operações aritméticas/lógicas. Flags zero, negativo, carry e overflow alimentam controle.",
"Clock discretiza mudanças. Flip-flops/registradores armazenam bits entre ciclos; circuitos sequenciais dependem de estado anterior.",
"CPU coordena fetch, decode e execute com PC, registradores, controle e datapath. Branch altera PC e load/store conecta memória.",
"ISA é contrato software-hardware: instruções, registradores, formatos, endereçamento e semântica. Microarquiteturas distintas podem implementar a mesma ISA.",
"Assembly x86-64 expõe registradores, stack, CALL/RET e ABI. Convenção de chamada define argumentos e registradores preservados.",
"Cache explora localidade em linhas. Miss compulsório, de capacidade e de conflito explicam comportamento; associatividade reduz conflitos.",
"Pipeline sobrepõe estágios. Hazards de dados, controle e recursos exigem forwarding, stalls ou previsão; throughput difere de latência.",
"Virtualização compartilha hardware entre sistemas isolados. Hypervisor e suporte de CPU/MMU controlam operações privilegiadas e memória."
],
"07":[
"Kernel roda privilegiado e controla recursos; user space é restrito. Syscalls são transições controladas para pedir serviços.",
"Processo reúne endereço virtual, descritores, credenciais e threads. Context switch salva estado; fork deriva e exec substitui programa.",
"Threads compartilham heap/descritores, mas têm stacks e registradores próprios. Compartilhamento rápido cria riscos de race.",
"Scheduler decide qual thread usa CPU. FCFS, SJF e Round Robin expõem trade-offs de throughput, latência e fairness.",
"Concorrência exige preservar invariantes de estado compartilhado. Mutex, semáforo e condition variable coordenam interleavings.",
"Deadlock depende de exclusão, hold-and-wait, ausência de preempção e espera circular. Prevenção quebra condição; detecção procura ciclos.",
"Memória virtual traduz endereços por page tables/TLB. Page faults transferem controle ao kernel e demand paging carrega sob necessidade.",
"Alocadores de user space administram heap e kernel administra páginas/frames. Free lists, buddy e slab atacam padrões diferentes.",
"Filesystem mapeia nomes para metadados/blocos. Inodes separam identidade de nome e journaling ajuda recuperação após crash.",
"I/O envolve dispositivos, interrupts e DMA. Drivers traduzem operações e buffering desacopla velocidades.",
"Syscalls como read/write/open/mmap/futex validam argumentos e atravessam privilégio; bibliotecas constroem APIs sobre essas primitivas.",
"Containers combinam namespaces, cgroups, mounts e capabilities. São processos isolados, não máquinas virtuais completas."
],
"08":[
"Camadas separam responsabilidades: link move frames locais, IP roteia pacotes, transporte liga processos e aplicação define semântica.",
"Ethernet usa frames/MAC numa LAN. Switch aprende MAC→porta e inunda destinos desconhecidos.",
"IP fornece datagramas best-effort e endereçamento roteável. CIDR define prefixos; routers usam longest-prefix match; TTL evita loops.",
"ARP resolve IPv4 local para MAC; IPv6 usa Neighbor Discovery. A resolução ocorre antes de enviar frame ao próximo salto local.",
"ICMP carrega controle/diagnóstico. Echo sustenta ping; Time Exceeded sustenta traceroute; erros ajudam hosts a reagir.",
"UDP adiciona portas/checksum sem conexão, retransmissão ou ordem. A aplicação aceita isso ou implementa garantias adicionais.",
"TCP cria stream confiável com sequence, ACK, retransmissão, flow e congestion control. Não preserva fronteiras de mensagens.",
"DNS é hierárquico, distribuído e cacheado. Resolvers percorrem root/TLD/autoritativos e TTL limita validade.",
"HTTP define mensagens e recursos. HTTP/1.1 usa framing textual, HTTP/2 multiplexa streams e HTTP/3 usa QUIC.",
"TLS autentica e deriva chaves de sessão para confidencialidade/integridade. Certificados formam cadeia de confiança.",
"Sockets expõem endpoints. bind/listen/accept servem TCP; connect inicia cliente; send/recv podem transferir menos bytes que pedidos.",
"NAT reescreve endereços/portas; firewall aplica política; proxy termina/inicia conexões e pode balancear, cachear e terminar TLS."
],
"09":[
"Modelo relacional usa relações, atributos e tuplas. Chaves identificam e foreign keys preservam referências; normalização reduz anomalias.",
"SQL é declarativa: descreve resultado. SELECT/JOIN/GROUP BY/CTE compõem consultas enquanto engine escolhe algoritmos.",
"Álgebra relacional formaliza seleção, projeção, produto, join, união e diferença, permitindo reescritas equivalentes.",
"Storage engine usa páginas fixas. Slotted pages separam diretório e payload para mover registros sem quebrar IDs lógicos.",
"Índices trocam espaço/escrita por leitura. B+ Tree favorece equality/range; hash favorece equality; seletividade decide utilidade.",
"Query processing materializa scans, joins, sorts e agregações. Cada operador consome/produz tuples ou blocos.",
"Optimizer estima cardinalidades/custos para escolher join order, índices e algoritmos. Estatísticas erradas geram planos ruins.",
"Transação agrupa operações com atomicidade, consistência, isolamento e durabilidade implementados por logging e controle de concorrência.",
"Locks controlam conflitos; MVCC mantém versões para reduzir bloqueio. Isolation levels definem anomalias permitidas.",
"WAL registra antes de páginas persistirem. Recovery aplica redo/undo conforme protocolo e checkpoints limitam trabalho.",
"NoSQL inclui key-value, documento, wide-column e graph. Escolha depende de acesso, consistência e escala, não de moda."
],
"10":[
"Implementar linguagem exige sintaxe, semântica e runtime. Pipeline pode interpretar AST, executar bytecode ou gerar nativo.",
"Lexer transforma caracteres em tokens e resolve palavras-chave, identificadores, números e operadores preservando posição para erros.",
"Parser verifica gramática e cria estrutura. Recursive descent e Pratt são técnicas úteis para precedência e associatividade.",
"AST remove detalhes puramente sintáticos e preserva estrutura semântica para passes de análise/execução.",
"Interpretador avalia nós em ambiente. Runtime errors ocorrem depois de sintaxe válida e precisam de localização/contexto.",
"Escopo léxico resolve nomes pela estrutura do código. Ambientes encadeados implementam blocos, shadowing e closures.",
"Tipos classificam valores/operações. Checagem estática antecipa erros; dinâmica verifica no runtime; inferência deduz tipos.",
"Bytecode é ISA virtual. VM stack-based simplifica encoding; dispatch interpreta opcodes, frames e operandos.",
"Compilação pode usar IR antes de assembly. IR facilita análise/otimização e separa frontend de backend.",
"Garbage collection recupera inalcançáveis. Reference counting sofre ciclos; tracing mark-and-sweep lida com ciclos.",
"Otimizações preservam semântica. Constant folding, DCE, inlining e CSE exigem provas/análises de segurança."
],
"11":[
"API é contrato. Pré/pós-condições, erros, idempotência e compatibilidade precisam ser explícitos para clientes.",
"Unit testa unidade, integration combina componentes, property-based testa invariantes e E2E valida fluxo completo.",
"Coesão alta agrupa responsabilidades e acoplamento baixo reduz dependências. SOLID é heurística, não objetivo absoluto.",
"Refatoração muda estrutura sem mudar comportamento. Testes e passos pequenos tornam regressões localizáveis.",
"Logs são eventos, métricas são séries e traces conectam spans. Observabilidade deve permitir perguntas não previstas.",
"Performance exige profile antes de otimizar. Latência, throughput e percentis de cauda respondem perguntas diferentes.",
"Build resolve dependências e produz artefatos reproduzíveis. Lockfiles, checksums e provenance protegem supply chain.",
"CI valida mudanças e CD automatiza promoção/deploy. Pipeline deve ser determinístico e proteger ambientes/segredos.",
"Versionamento comunica compatibilidade. SemVer, migrations e deprecation ajudam evolução sem quebra abrupta.",
"Arquitetura define fronteiras. Monólito modular reduz complexidade operacional; microservices adicionam rede e consistência distribuída."
],
"12":[
"Concorrência trata tarefas em progresso; paralelismo executa simultaneamente. Um sistema pode ter um sem o outro.",
"Threads compartilham heap mas têm stacks/registros próprios. Context switch e sincronização têm custo.",
"Locks protegem seção crítica; atomics oferecem operações indivisíveis. Memory ordering define reordenações observáveis.",
"Semáforo conta permissões; condition variable dorme até condição mudar e deve ser usada com mutex e rechecagem em loop.",
"Lock-free usa atomics como compare-and-swap para progresso global. ABA, reclamation e ordering tornam implementação difícil.",
"Async/await estrutura concorrência cooperativa; event loop alterna tarefas durante espera de I/O. Bloquear o loop destrói escalabilidade.",
"Multiprocessing separa memória e usa IPC. Custa serialização, mas isola falhas e usa múltiplos cores em runtimes com GIL.",
"SIMD aplica operação a múltiplos elementos. Layout, alinhamento e dependências decidem se vetorização é possível."
],
"13":[
"Modelo de falha define o que tolerar: crash-stop, recovery, omission, partition ou Byzantine. Garantia depende do modelo.",
"Relógios físicos não sincronizam perfeitamente. Lamport captura ordem causal parcial e vector clocks distinguem concorrência.",
"Replicação aumenta disponibilidade. Leader-based ordena facilmente; multi-leader/leaderless mudam conflitos e reconciliação.",
"Sharding divide por chave. Range favorece scans e pode gerar hotspot; hash distribui melhor e complica range.",
"Consistência define observações permitidas: linearizability, sequential e eventual têm garantias e custos diferentes.",
"CAP: sob partição, disponibilidade total e consistência linearizável não coexistem. PACELC adiciona trade-off latência/consistência sem partição.",
"Consensus produz sequência comum de decisões. Quorums intersectantes preservam informação; Raft usa termos, líder e log.",
"Leader election precisa evitar split brain. Epoch/term permite rejeitar líderes antigos e mensagens obsoletas.",
"Transação distribuída coordena participantes. 2PC pode bloquear; sagas usam passos e compensações.",
"Queues desacoplam; logs append-only preservam ordem por partição/replay. Delivery semantics dependem de efeitos externos.",
"MapReduce divide map e reduce; shuffle redistribui por chave e frequentemente domina I/O."
],
"14":[
"Threat modeling identifica ativos, fronteiras, adversários e impactos antes dos controles. STRIDE ajuda a enumerar ameaças.",
"Criptografia simétrica usa segredo compartilhado. AEAD como AES-GCM/ChaCha20-Poly1305 combina confidencialidade e autenticação.",
"Hash não tem chave; MAC autentica com segredo; KDF deriva chaves. Password hashing usa função cara e salt único.",
"Assimétrica usa chave pública/privada. Protocolos híbridos usam key agreement/assinatura para estabelecer chaves simétricas.",
"Assinatura usa privada para assinar e pública para verificar. Contexto e nonces adequados evitam reutilização perigosa.",
"PKI liga identidade a chave por certificados e CAs. Validação inclui cadeia, hostname, validade, política e revogação.",
"TLS negocia parâmetros, autentica e deriva chaves de tráfego; registros AEAD protegem a aplicação.",
"Authentication prova identidade; authorization decide permissão. Sessions, OAuth e OIDC resolvem partes distintas.",
"Web security cobre XSS, CSRF, injection, SSRF e sessão. Cada ameaça exige controle apropriado, não uma defesa universal.",
"Memory safety impede acesso fora de bounds/lifetime. Rust verifica ownership/borrowing estaticamente, mas não elimina falhas lógicas.",
"Sandboxing reduz privilégios com isolamento, seccomp, namespaces, capabilities e políticas.",
"Secure coding inclui validação, least privilege, segredos, logs seguros, atualização e design fail-safe."
],
"15":[
"URL→pixel passa por URL parsing, DNS, conexão, TLS, HTTP, HTML/CSS, JS, layout, paint e compositing.",
"HTML forma DOM semântico. Elementos corretos melhoram estrutura/acessibilidade e parser segue regras mesmo com markup imperfeito.",
"CSS usa cascade, inheritance e specificity. Flexbox é unidimensional, Grid bidimensional; mudanças podem provocar reflow.",
"JavaScript usa call stack, event loop, task queues e microtasks. Promise agenda continuação sem bloquear I/O.",
"Backend faz routing, middleware, regras e dados. Separar HTTP, domínio e persistência reduz acoplamento.",
"REST modela recursos, RPC chamadas e GraphQL seleção de campos. Cada estilo troca simplicidade, controle e acoplamento.",
"Cache reduz latência/carga e cria invalidação. Cache-Control, ETag e CDN operam em camadas; chave/TTL precisam refletir variantes.",
"WebSocket mantém conexão bidirecional. Realtime exige heartbeat, backpressure, reconexão e escala de conexões."
],
"16":[
"Linguagem formal é conjunto de strings sobre alfabeto. Reconhecedores decidem pertinência e geradores descrevem strings válidas.",
"DFA tem uma transição por símbolo; NFA permite múltiplas/ε. Ambos reconhecem linguagens regulares e subset construction converte NFA→DFA.",
"Regex formal usa união, concatenação e estrela para linguagens regulares. Engines modernas podem acrescentar recursos não regulares.",
"CFG usa produções A→α e descreve estruturas aninhadas. Ambiguidade ocorre quando uma string tem múltiplas derivações estruturais.",
"Máquina de Turing usa fita, cabeça, estados e transição; serve como modelo universal de computabilidade.",
"Computabilidade separa funções com algoritmo das impossíveis genericamente. Enumerabilidade/reduções comparam poder.",
"Problema da parada não possui decisor geral; a prova usa auto-referência para obter contradição.",
"P contém decisão polinomial e NP certificados verificáveis em tempo polinomial. Ainda não se sabe P=NP.",
"Redução polinomial transforma A em B preservando resposta. Para provar B difícil, reduz-se problema difícil conhecido a B."
],
"17":[
"Gráficos usa vetores/matrizes/projeções. Rasterization converte primitivas em fragments; ray tracing resolve interseções de raios.",
"IA clássica inclui busca, CSP, minimax e heurísticas. Representação de estado e função de custo dominam desempenho.",
"ML começa com loss e otimização. Gradiente dá direção local e backprop aplica regra da cadeia em DAG de operações.",
"Storage avançado usa LSM para writes sequenciais, Bloom para negativas e compaction com write amplification.",
"Cloud combina virtualização, rede, storage e automação. Load balancing, discovery, autoscaling e orchestration lidam com mudança.",
"Rust usa ownership, borrowing e lifetimes para verificar várias classes de memória sem GC.",
"Information retrieval usa inverted index e ranking. TF-IDF/BM25 ponderam frequência/raridade e analyzers normalizam texto.",
"Compressão explora redundância. Huffman usa códigos prefix-free; LZ usa referências; entropy limita custo médio sob modelo."
],
"18":[
"Mini computador integra portas, registradores, ALU, memória, CPU e assembler para executar programa explicável bit a bit.",
"Mini SO integra boot, interrupções, scheduler, memória e filesystem mínimos em hardware/emulador.",
"Banco capstone combina páginas, B+ Tree, SQL, executor, transações e WAL, incluindo recovery após restart.",
"Redis-like foca protocolo, event loop, estruturas em memória, expiração e persistência sob baixa latência.",
"Git-like integra content-addressed storage, trees, commits, refs, checkout e merge por ancestralidade.",
"Container runtime usa namespaces, cgroups, mounts e init para compor isolamento com primitivas Linux.",
"Linguagem integra lexer, parser, AST, nomes, runtime/VM e GC opcional sob uma especificação.",
"Distributed KV store combina rede, sharding, replicação, persistência e consenso opcional, testando falhas.",
"Search engine combina tokenizer, inverted index, ranking, persistência e API, medindo relevância e latência.",
"Stack web completo conecta navegador, API, auth, banco, cache e observabilidade e documenta request path e ameaças."
]
}

FORMULAS={
("03","Combinatória"):"C(n,k) = n! / (k!(n-k)!)",
("03","Probabilidade discreta"):"P(A|B) = P(B|A)P(A) / P(B)",
("03","Recorrências"):"T(n) = aT(n/b) + f(n)",
("05","Análise assintótica"):"T(n) pertence a Theta(g(n)) quando fica entre constantes positivas vezes g(n), para n suficientemente grande.",
("17","ML por dentro"):"theta <- theta - eta * grad L(theta)"
}

def slugify(text):
    text=unicodedata.normalize("NFKD",text).encode("ascii","ignore").decode().lower()
    return re.sub(r"[^a-z0-9]+","-",text).strip("-") or "aula"

def lesson(module,index,chapter,focus):
    mid=module["id"];formula=FORMULAS.get((mid,chapter))
    fpart="" if not formula else "\n## Fórmula / propriedade central\n\n"+formula+"\n"
    return f"""# {index:02d} — {chapter}

Módulo {mid}: {module['title']} · Nível: {module['level']}

## Ideia central

{focus}

## Modelo mental

{MODULE_OVERVIEW[mid]}

Neste capítulo identifique estado, invariante, operação e custo. Se houver uma abstração, pergunte qual problema ela resolve e o que acontece uma camada abaixo.
{fpart}
## Código e laboratório

O laboratório principal do módulo está em [{LABS[mid]}](../{LABS[mid]}). Execute os testes antes de alterar o código.

Ciclo de investigação:

1. execute a versão atual;
2. formule uma hipótese;
3. altere uma variável, estrutura ou entrada;
4. observe teste, saída, tempo, memória ou estado;
5. explique causalmente o resultado.

## Experimento guiado

1. Escolha uma entrada pequena simulável à mão.
2. Registre o estado antes de cada passo.
3. Execute e compare com sua previsão.
4. Crie um caso extremo: vazio, limite, repetido, inválido ou grande.
5. Reduza qualquer divergência até o primeiro passo inesperado.

## Perguntas de domínio

- Qual é a definição operacional de {chapter}?
- Que invariante ou garantia é essencial?
- Que custo de tempo, espaço, coordenação ou I/O cresce com a entrada?
- O que a abstração esconde da camada inferior?
- Que falha aparece se uma hipótese central deixar de valer?

## Exercícios

1. Explique o conceito em cinco frases sem consultar a aula.
2. Crie um exemplo correto e um contraexemplo.
3. Torne o conceito observável no laboratório do módulo.
4. Justifique o resultado com vocabulário técnico.
5. Conecte este capítulo ao anterior e ao próximo.

## Critério de conclusão

Você concluiu quando consegue prever um caso novo, explicar o mecanismo por baixo da API e justificar pelo menos um trade-off relevante.
"""

def main():
    curriculum=json.loads(CURRICULUM.read_text(encoding="utf-8"))
    site=[];total=0
    for module in curriculum:
        mid=module["id"];focus=FOCUS[mid]
        if len(focus)!=len(module["chapters"]):
            raise RuntimeError(f"{mid}: focos={len(focus)} capitulos={len(module['chapters'])}")
        base=ROOT/module["slug"];adir=base/"aulas";adir.mkdir(parents=True,exist_ok=True)
        lines=[f"# Aulas — Módulo {mid}: {module['title']}","",MODULE_OVERVIEW[mid],"",f"Laboratório principal: [{LABS[mid]}](./{LABS[mid]})","","## Sequência",""]
        for i,(chapter,note) in enumerate(zip(module["chapters"],focus),1):
            filename=f"{i:02d}-{slugify(chapter)}.md"
            (adir/filename).write_text(lesson(module,i,chapter,note),encoding="utf-8")
            lines.append(f"{i}. [{chapter}](./aulas/{filename})")
            path=f"{module['slug']}/aulas/{filename}"
            site.append({"module":mid,"moduleTitle":module["title"],"title":chapter,"description":note,"path":path,"url":REPO_BASE+path})
            total+=1
        (base/"AULAS.md").write_text("\n".join(lines)+"\n",encoding="utf-8")
    LESSONS_JSON.write_text(json.dumps(site,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    (ROOT/"AULAS_COMPLETAS.md").write_text(f"# Aulas completas\n\nForam geradas **{total} aulas**, cobrindo todos os capítulos da super ementa. Cada módulo possui AULAS.md e pasta aulas/, ligadas aos projetos executáveis.\n",encoding="utf-8")
    print(f"Geradas {total} aulas.")

if __name__=="__main__":
    main()
