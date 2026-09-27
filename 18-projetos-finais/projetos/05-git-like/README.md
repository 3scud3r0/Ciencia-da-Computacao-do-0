# Capstone 05 — Git-like

Armazenamento content-addressed com blobs, trees, commits, refs de branches, `HEAD`, switch e diff entre commits.

O histórico é um DAG por ponteiro `parent`; branches são arquivos que apontam para commits. O mesmo conteúdo gera o mesmo object id.

A implementação não tenta ser byte-a-byte compatível com Git: o objetivo é reconstruir o modelo de dados e as operações fundamentais.
