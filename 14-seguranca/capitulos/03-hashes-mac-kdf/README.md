# Hashes, MAC e KDF

Essas três ferramentas parecem semelhantes porque produzem bytes a partir de dados, mas resolvem problemas diferentes.

## Hash criptográfico

`H(message) → digest`.

Propriedades desejadas incluem resistência a preimage, second preimage e colisões. Hash não é criptografia reversível.

## MAC

Um Message Authentication Code inclui uma chave secreta:

`MAC(key, message) → tag`.

Quem possui a chave pode verificar integridade **e autenticidade**. Um hash simples não prova quem produziu a mensagem.

HMAC combina uma função hash com uma construção específica para MAC; não é equivalente a `hash(key + message)`.

## KDF

Key Derivation Functions transformam material secreto em chaves apropriadas. Password hashing é um caso especializado: Argon2, scrypt e bcrypt são deliberadamente caros para tornar ataques de tentativa em massa mais custosos.

## Salt

Salt não precisa ser secreto. Ele torna hashes de senhas iguais diferentes entre usuários e impede tabelas pré-computadas universais.

## Regra de engenharia

Não invente primitivas criptográficas para produção. Implementações didáticas servem para compreender interfaces e propriedades; software real deve usar bibliotecas revisadas e configurações modernas.
