# Capstone-semente: KV store persistente com HTTP

Este projeto integra conceitos de várias camadas:

- **storage:** log append-only (WAL);
- **serialização:** JSON lines;
- **concorrência:** lock protegendo estado e escrita;
- **rede/web:** HTTP GET/PUT;
- **recovery:** replay do WAL ao reiniciar.

É pequeno, mas não é um mock: grave `PUT /kv/chave` com JSON `{"value": ...}`, finalize o processo e reinicie; o estado é reconstruído do log.

Próximos passos: checksums, compaction, índice em memória, snapshots, replicação e consenso.
