from __future__ import annotations
from collections import deque
from dataclasses import dataclass,field

@dataclass
class Process:
    pid:int
    program:list[tuple[str,int]]
    pc:int=0
    remaining:int=0
    state:str="ready"
    wake_at:int=0
    cpu_time:int=0
    trace:list[str]=field(default_factory=list)

class Kernel:
    def __init__(self,quantum=2):
        if quantum<=0:raise ValueError("quantum > 0")
        self.quantum=quantum;self.clock=0;self.next_pid=1;self.processes={};self.ready=deque()
    def spawn(self,program):
        pid=self.next_pid;self.next_pid+=1
        p=Process(pid,list(program));self.processes[pid]=p;self.ready.append(pid);return pid
    def _wake(self):
        for p in self.processes.values():
            if p.state=="blocked" and p.wake_at<=self.clock:
                p.state="ready";self.ready.append(p.pid);p.trace.append(f"{self.clock}:wake")
    def run(self,max_ticks=10000):
        ticks=0
        while any(p.state!="done" for p in self.processes.values()):
            if ticks>=max_ticks:raise RuntimeError("limite excedido")
            self._wake()
            if not self.ready:
                next_wake=min(p.wake_at for p in self.processes.values() if p.state=="blocked")
                self.clock=next_wake;self._wake();continue
            pid=self.ready.popleft();p=self.processes[pid]
            if p.state!="ready":continue
            p.state="running";budget=self.quantum
            while budget>0 and p.state=="running":
                if p.pc>=len(p.program):p.state="done";p.trace.append(f"{self.clock}:exit");break
                op,arg=p.program[p.pc]
                if op=="CPU":
                    if p.remaining==0:p.remaining=arg
                    p.remaining-=1;p.cpu_time+=1;self.clock+=1;budget-=1;ticks+=1
                    p.trace.append(f"{self.clock}:cpu")
                    if p.remaining==0:p.pc+=1
                    self._wake()
                elif op=="IO":
                    p.pc+=1;p.state="blocked";p.wake_at=self.clock+arg;p.trace.append(f"{self.clock}:io({arg})")
                elif op=="YIELD":
                    p.pc+=1;p.trace.append(f"{self.clock}:yield");break
                else:raise ValueError(op)
            if p.state=="running":
                if p.pc>=len(p.program):p.state="done";p.trace.append(f"{self.clock}:exit")
                else:p.state="ready";self.ready.append(pid)
        return self.clock
