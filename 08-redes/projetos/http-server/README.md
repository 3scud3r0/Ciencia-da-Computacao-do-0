# Servidor HTTP/1.1 mínimo

Aqui não existe framework: o programa abre um **socket TCP**, aceita conexão, lê bytes, localiza `\r\n\r\n`, interpreta request line/headers e envia bytes de resposta.

~~~text
cliente → TCP → accept → recv → parse_request → handle → response → sendall
~~~

A implementação é propositalmente incompleta: uma requisição por conexão, sem chunked encoding, keep-alive ou HTTP/2. Cada limitação vira um exercício para entender por que servidores reais são maiores.
