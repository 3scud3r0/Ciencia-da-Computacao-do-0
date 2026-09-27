# Módulo 07 — Sistemas operacionais

**Nível:** Profundo  
**Carga editorial:** 26–40h  
**Ferramentas:** C · Linux/POSIX

## Resultado esperado

Este módulo existe para formar um modelo mental, não para decorar APIs. Ao concluí-lo, você deve responder com precisão:

- Como o kernel compartilha CPU, memória e dispositivos?
- O que acontece em syscalls, processos e page faults?

## Capítulos

1. **Kernel e user space**
2. **Processos**
3. **Threads**
4. **Scheduling**
5. **Concorrência**
6. **Deadlocks**
7. **Memória virtual**
8. **Alocação**
9. **Filesystems**
10. **I/O**
11. **Syscalls**
12. **Containers**

## Projetos

- Scheduler simulator
- Shell POSIX
- Filesystem
- Container mínimo

## Protocolo de estudo

1. Entenda o problema que motivou a abstração.
2. Execute a menor implementação possível.
3. Modifique o código e provoque um erro intencional.
4. Rode testes, debugger e medições quando aplicáveis.
5. Explique o mecanismo sem usar “mágica” como resposta.
6. Resolva um exercício a partir de uma folha em branco.
7. Faça a conexão com a camada anterior e a próxima.

## Critério de domínio

O módulo só está concluído quando você consegue explicar os conceitos centrais, reproduzir as implementações pequenas, prever casos extremos, justificar trade-offs e depurar uma versão defeituosa.

## Biblioteca externa

Use `../vendor/` e o catálogo de projetos do site como laboratório adicional. Soluções externas servem para comparação; os mecanismos fundamentais também terão implementações próprias neste curso.
