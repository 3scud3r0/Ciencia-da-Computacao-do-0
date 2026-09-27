# Capstone 06 — Container runtime mínimo

Runtime Linux que compõe namespaces de mount, UTS, IPC e PID usando `unshare`, monta `/proc` no namespace novo e permite limites de memória/CPU via `setrlimit`.

O teste valida a especificação sem depender de privilégios do runner. Em Linux que permite user namespaces/unshare, `ContainerRuntime.run()` executa o comando isolado nesses namespaces.

Isto demonstra a mecânica central; um runtime de produção ainda precisa rootfs/pivot_root, cgroups, capabilities, seccomp, networking e lifecycle robusto.
