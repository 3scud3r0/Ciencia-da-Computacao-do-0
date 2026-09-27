# Simulador de scheduler

Compare três políticas sobre a mesma carga:

- **FCFS**: executa por ordem de chegada;
- **SJF não preemptivo**: escolhe o menor burst entre jobs disponíveis;
- **Round Robin**: divide CPU em quantums e recoloca jobs incompletos na fila.

As métricas são:

`turnaround = completion - arrival`

`waiting = turnaround - burst`

O objetivo não é declarar um algoritmo universalmente melhor: cargas interativas, batch, prioridades e fairness exigem trade-offs diferentes. Altere os bursts e observe quando SJF reduz espera média, mas prejudica jobs longos; depois varie o quantum do Round Robin.
