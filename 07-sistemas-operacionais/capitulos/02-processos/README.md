# Processos

Processo é a abstração que faz um programa se comportar como se tivesse uma máquina própria.

O kernel associa a cada processo:

- espaço de endereçamento virtual;
- uma ou mais threads;
- descritores de arquivos;
- credenciais e permissões;
- estado necessário para scheduling e sinais.

## Context switch

Uma CPU executa uma thread por core de cada vez. Para alternar, o kernel salva estado suficiente da thread atual e restaura o estado de outra. Esse mecanismo produz a ilusão de simultaneidade mesmo quando há mais tarefas que cores.

## fork e exec

Em Unix, `fork()` cria um novo processo derivado do atual. `exec()` substitui o programa daquele processo por outro executável.

Esse desenho explica a implementação clássica de um shell:

~~~text
shell
  ├─ fork → filho
  │          └─ exec("programa")
  └─ wait ← aguarda
~~~

## Isolamento não é absoluto

Processos têm espaços virtuais separados, mas compartilham kernel, CPU, caches, dispositivos e podem compartilhar explicitamente memória/arquivos/sockets.

## Experimento

Use `ps`, `/proc/<pid>/maps`, `strace` e `lsof` para observar o processo como objeto do sistema operacional.

A pergunta central é: qual estado pertence ao programa em user space e qual estado o kernel mantém em seu nome?
