# HTTP por baixo do framework

HTTP é um protocolo de aplicação. Em HTTP/1.1 clássico, uma mensagem textual contém request line/status line, headers, linha vazia e possivelmente body.

~~~text
GET /health HTTP/1.1\r\n
Host: example.com\r\n
Accept: */*\r\n
\r\n
~~~

O servidor responde com algo como:

~~~text
HTTP/1.1 200 OK
Content-Length: 3
Content-Type: text/plain

ok
~~~

## Por que Content-Length importa

TCP não possui “fim da mensagem HTTP”. A aplicação precisa saber quantos bytes pertencem ao body ou usar outro mecanismo de framing.

## Métodos e semântica

GET, POST, PUT, DELETE etc. são parte do protocolo, não funções mágicas de um framework. Status codes descrevem o resultado em categorias 1xx–5xx.

## Evolução

HTTP/2 muda o framing e multiplexa streams em uma conexão. HTTP/3 transporta HTTP sobre QUIC/UDP. A semântica de alto nível permanece reconhecível, embora os bytes no fio sejam muito diferentes.

Execute `projetos/http-server`, depois use `curl -v` e observe exatamente o que foi enviado e recebido.
