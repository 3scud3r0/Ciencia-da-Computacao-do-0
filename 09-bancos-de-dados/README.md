# Módulo 09 — Bancos de dados

**Nível:** Profundo  
**Carga editorial:** 22–36h  
**Ferramentas:** SQL · Python/C

## Resultado esperado

Este módulo existe para formar um modelo mental, não para decorar APIs. Ao concluí-lo, você deve responder com precisão:

- Como um banco encontra uma linha sem percorrer tudo?
- Como índices, MVCC e WAL preservam desempenho e consistência?

## Capítulos

1. **Modelo relacional**
2. **SQL**
3. **Álgebra relacional**
4. **Storage pages**
5. **Índices**
6. **Query processing**
7. **Optimizer**
8. **Transações**
9. **MVCC e locks**
10. **Recovery/WAL**
11. **NoSQL**

## Projetos

- Parser SQL
- Storage engine
- B+ Tree
- Mini banco relacional

## Protocolo de estudo

1. Entenda o problema que motivou a abstração.
2. Execute a menor implementação possível.
3. Modifique o código e provoque um erro intencional.
4. Rode testes, debugger e medições quando aplicáveis.
5. Explique o mecanismo sem usar “mágica” como resposta.
6. Resolva um exercício a partir de uma folha em branco.
7. Faça a conexão com a camada anterior e a próxima.

## Critério de domínio

O módulo só está concluído quando você consegue explicar os conceitos centrais, reproduzir as implementações pequenas, prever casos extremos, justificar trade-offs e depurar uma versão defeituosa.

## Biblioteca externa

Use `../vendor/` e o catálogo de projetos do site como laboratório adicional. Soluções externas servem para comparação; os mecanismos fundamentais também terão implementações próprias neste curso.
