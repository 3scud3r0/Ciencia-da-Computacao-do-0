# Capstone 08 — Distributed KV store

Cluster em memória com particionamento determinístico por hash, replication factor, read/write quorum, versões monotônicas, simulação de node failure e read repair.

O projeto torna visível a diferença entre “gravei em um nó” e “obtive quorum”. Uma réplica que ficou offline pode voltar desatualizada; a leitura escolhe a versão mais nova e repara réplicas antigas.

Próximos níveis: hinted handoff, vector clocks, persistent WAL, consistent hashing com vnodes e consensus para uma semântica linearizável.
