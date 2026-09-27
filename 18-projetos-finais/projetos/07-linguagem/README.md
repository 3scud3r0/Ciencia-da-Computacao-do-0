# Capstone 07 — Linguagem com bytecode VM

Pipeline completo: tokenizer → parser recursive-descent → AST → compilador → bytecode stack-based → VM.

A linguagem suporta `let`, `print`, variáveis, parênteses, precedência, aritmética e negação. O compilador transforma árvore em instruções `CONST/LOAD/STORE/ADD/.../PRINT/HALT`.

Próximos níveis: funções, call frames, jumps/if/while, closures, objetos, type checker e garbage collector.
