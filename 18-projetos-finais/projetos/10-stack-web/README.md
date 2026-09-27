# Capstone 10 — Stack web completo

Aplicação com banco SQLite, schema relacional, password hashing por scrypt, sessions com expiração, autenticação Bearer, regras de domínio, CRUD de notas e camada de routing HTTP-independente.

O fluxo testado cobre registro → login → sessão → criação/leitura de dados → expiração da sessão. Queries usam parâmetros SQL em vez de concatenação.

Para transformar em serviço web real, conecte `route()` a um servidor HTTP, adicione CSRF/cookies conforme o modelo, observabilidade, migrations e cache.
