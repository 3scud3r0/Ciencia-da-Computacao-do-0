# Password hashing com scrypt

Senhas não devem ser armazenadas em texto puro nem com hash rápido simples. Este laboratório usa `hashlib.scrypt`, salt aleatório e `hmac.compare_digest`.

O registro salva algoritmo + parâmetros + salt + derivação. Assim os parâmetros podem evoluir e cada usuário recebe salt diferente.

Isto é um exemplo didático de armazenamento de senha; aplicações reais devem usar bibliotecas/frameworks maduros, políticas de atualização de parâmetros e proteção operacional adicional.
