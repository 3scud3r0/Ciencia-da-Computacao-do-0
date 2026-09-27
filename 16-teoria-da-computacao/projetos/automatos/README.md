# NFA, ε-closure e determinização

Um NFA pode estar conceitualmente em vários estados ao mesmo tempo. Transições ε mudam de estado sem consumir símbolo.

`epsilon_closure(S)` calcula todos os estados alcançáveis somente por ε. Para consumir um símbolo, o simulador expande a closure, segue todas as transições daquele símbolo e calcula nova closure.

A **subset construction** transforma cada conjunto de estados NFA em um estado DFA. O teste prova empiricamente, para várias strings, que o DFA determinizado preserva a linguagem do NFA de exemplo `a*b`.

A próxima etapa é implementar Thompson construction para converter expressões regulares em NFA.
