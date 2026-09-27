# Ponteiros sem misticismo

Um ponteiro é um valor usado como endereço de memória. Em C, o tipo também informa como interpretar os bytes encontrados naquele endereço.

~~~c
int x = 42;
int *p = &x;
*p = 99;
~~~

`&x` produz o endereço de `x`. `p` guarda esse endereço. `*p` acessa o objeto apontado. Depois da atribuição, `x == 99`.

## Por que o tipo importa

Se `p` é `int *`, `p + 1` não soma um byte: avança `sizeof(int)` bytes. Pointer arithmetic é definida em termos de elementos de um array.

~~~text
p       → data[0]
p + 1   → data[1]
p + 2   → data[2]
~~~

## Ponteiro não é objeto

O endereço pode continuar existindo numericamente quando o objeto já deixou de existir. Acessá-lo então é inválido.

~~~c
int *p = malloc(sizeof *p);
free(p);
/* *p agora seria use-after-free */
~~~

`free` libera a alocação; não apaga automaticamente todas as cópias do endereço.

## Stack e heap

Variáveis locais normalmente pertencem ao frame da chamada. Alocações com `malloc` possuem duração controlada explicitamente. Essa diferença explica bugs como retornar endereço de variável local, leaks e double free.

## Experimento

No GDB, pare antes e depois de uma atribuição via ponteiro:

~~~text
break main
run
print &x
print p
x/4xb p
~~~

O objetivo é perceber que “referência”, “objeto” e “endereço” são conceitos relacionados, mas diferentes.

Pratique no projeto `../projetos/vetor-dinamico/`, onde um ponteiro para um bloco real é substituído quando `realloc` move a alocação.
