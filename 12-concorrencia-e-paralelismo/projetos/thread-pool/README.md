# Thread pool

Criar uma thread por tarefa tem custo. Um pool mantém workers vivos e distribui jobs por uma fila thread-safe.

`Future` desacopla submissão e resultado. O worker captura exceções e as transfere ao Future; a thread chamadora recebe o erro ao chamar `result()`.

Compare 1/2/4/8 workers em tarefas I/O-bound e depois CPU-bound em CPython. A diferença leva diretamente ao estudo do GIL e de multiprocessing.
