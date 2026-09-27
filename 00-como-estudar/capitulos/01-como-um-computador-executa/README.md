# Como um computador executa um programa

Quando você executa `python app.py`, várias camadas entram em ação. O shell resolve o nome `python` usando o `PATH`; o sistema operacional cria um processo; o executável do interpretador é mapeado para memória virtual; bibliotecas são carregadas; a CPU começa a executar instruções na entrada do processo; por fim o interpretador abre `app.py`, transforma seu texto em estruturas internas e executa o programa.

## O modelo mínimo

~~~text
arquivo-fonte
   ↓
programa que lê/compila/interpreta
   ↓
processo
   ↓
memória virtual + descritores de arquivo
   ↓
instruções de máquina
   ↓
CPU + caches + RAM + dispositivos
~~~

Um **programa** é uma descrição armazenada. Um **processo** é uma instância em execução, com estado: registradores, memória virtual, arquivos abertos, credenciais, threads e metadados do kernel.

A CPU não entende Python, C ou JavaScript. Ela entende a ISA da máquina: instruções que manipulam registradores, memória e controle de fluxo. C costuma ser compilado antecipadamente para essas instruções. CPython, por outro lado, compila o fonte Python para bytecode e uma máquina virtual em C executa esse bytecode.

## Experimento

No Linux:

~~~bash
which python
file "$(which python)"
python -c 'import os; print(os.getpid())'
strace -f python -c 'print("oi")'
~~~

`strace` mostra a fronteira entre user space e kernel: `openat`, `read`, `mmap`, `write` e outras syscalls.

## O que dominar

Você deve diferenciar: código-fonte, executável, processo, thread, bytecode, instrução de máquina, syscall e biblioteca. Essa distinção será reutilizada em todos os módulos seguintes.
