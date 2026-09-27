# Capstone 02 — Mini sistema operacional (simulador)

Kernel educacional executável com processos, estados `ready/running/blocked/done`, Round Robin, quantum, chamadas CPU/IO, bloqueio, wakeup e trace temporal.

Ele não inicializa hardware real: o escopo deste capstone é reproduzir os mecanismos de scheduling e bloqueio de forma observável e testável. A evolução natural é portar essas ideias para xv6/QEMU.
