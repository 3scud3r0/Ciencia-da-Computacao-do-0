# TCP

IP entrega datagramas sem prometer ordem, não duplicação ou chegada. TCP constrói sobre IP um fluxo de bytes confiável entre dois endpoints.

## Sequência

Cada byte pertence a uma posição lógica no fluxo. Sequence numbers permitem detectar dados ausentes, reordenar segmentos e confirmar recebimento.

## ACK e retransmissão

O receptor informa até onde recebeu dados. Se o emissor não obtém confirmação dentro de critérios temporais, pode retransmitir.

## Flow control vs congestion control

São problemas diferentes.

**Flow control** evita que o emissor sobrecarregue o buffer do receptor.

**Congestion control** tenta evitar que o emissor sobrecarregue a rede no caminho.

## Handshake

O three-way handshake sincroniza estado inicial:

~~~text
cliente → SYN       → servidor
cliente ← SYN + ACK ← servidor
cliente → ACK       → servidor
~~~

Ele não existe apenas para “testar conectividade”; estabelece números iniciais de sequência e confirma que ambos conseguem trocar segmentos.

## TCP não preserva mensagens

Do ponto de vista da aplicação, TCP é um **stream de bytes**. Um `send()` de 100 bytes não garante um `recv()` de 100 bytes. Protocolos de aplicação precisam definir framing: comprimento, delimitadores ou outra codificação.

Essa propriedade é decisiva no servidor HTTP do módulo.
