# Capstone 04 — Redis-like

Key-value store em memória com `SET`, `GET`, `DEL`, `INCR`, TTL/expiração e codificação RESP mínima.

TTL usa relógio injetável para testes determinísticos. O próximo passo é um event loop TCP que parseia RESP e persiste comandos em AOF.
