# Capstone 03 — Mini banco de dados

Storage persistente por WAL append-only, índice ordenado em memória, transações explícitas com commit/rollback e recovery por replay.

O arquivo em disco contém somente commits completos; mudanças de uma transação não são aplicadas ao estado visível antes do commit. O índice ordenado permite range scans por `bisect`.

Próximos níveis: páginas binárias, B+ Tree persistente, checksums, snapshots/compaction e MVCC.
