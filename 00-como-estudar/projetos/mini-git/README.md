# Mini Git: armazenamento content-addressed

Este laboratório implementa o núcleo conceitual do Git: objetos identificados pelo conteúdo.

O object id é SHA-1 de:

~~~text
"<tipo> <tamanho>\0" + payload
~~~

O objeto comprimido é salvo em `.minigit/objects/aa/bbbbb...`, exatamente a ideia de fan-out usada pelo Git. Dois blobs idênticos produzem o mesmo ID e são armazenados uma vez.

Nossa `tree` usa JSON didático em vez do formato binário real; o objetivo é separar **modelo conceitual** de compatibilidade byte-a-byte. O commit referencia a tree e um parent opcional, formando um DAG de histórico.

Execute `python -m unittest -v`. Depois implemente branches como arquivos que apontam para commits e um comando de checkout que materialize uma tree.
