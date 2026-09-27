# Serviço instrumentado

Um exemplo pequeno de engenharia de software: regra de negócio separada do armazenamento, dependência injetada e métricas observáveis.

O `UserService` depende de uma interface comportamental simples (`get/save`), não de um banco específico. O relógio também é injetável, permitindo testes determinísticos de latência.

O exercício é trocar `UserRepository` por uma implementação SQLite sem modificar as regras de `register()`.
