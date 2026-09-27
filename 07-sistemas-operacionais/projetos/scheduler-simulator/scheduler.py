from dataclasses import dataclass
from collections import deque
@dataclass(frozen=True,slots=True)
class Job:name:str;arrival:int;burst:int
@dataclass(frozen=True,slots=True)
class Slice:job:str;start:int;end:int
def _validate(jobs):
 names=set()
 for j in jobs:
  if j.name in names:raise ValueError('nomes duplicados')
  if j.arrival<0 or j.burst<=0:raise ValueError('arrival >= 0 e burst > 0')
  names.add(j.name)
def metrics(jobs,timeline):
 finish={}
 for s in timeline:finish[s.job]=s.end
 return {j.name:{'completion':finish[j.name],'turnaround':finish[j.name]-j.arrival,'waiting':finish[j.name]-j.arrival-j.burst} for j in jobs}
def fcfs(jobs):
 _validate(jobs);time=0;timeline=[]
 for j in sorted(jobs,key=lambda x:(x.arrival,x.name)):
  time=max(time,j.arrival);timeline.append(Slice(j.name,time,time+j.burst));time+=j.burst
 return timeline,metrics(jobs,timeline)
def sjf(jobs):
 _validate(jobs);pending=sorted(jobs,key=lambda x:(x.arrival,x.name));ready=[];time=0;timeline=[]
 while pending or ready:
  while pending and pending[0].arrival<=time:ready.append(pending.pop(0))
  if not ready:time=pending[0].arrival;continue
  j=min(ready,key=lambda x:(x.burst,x.arrival,x.name));ready.remove(j);timeline.append(Slice(j.name,time,time+j.burst));time+=j.burst
 return timeline,metrics(jobs,timeline)
def round_robin(jobs,quantum=2):
 _validate(jobs)
 if quantum<=0:raise ValueError('quantum > 0')
 pending=sorted(jobs,key=lambda x:(x.arrival,x.name));ready=deque();remaining={j.name:j.burst for j in jobs};time=0;timeline=[]
 while pending or ready:
  if not ready and pending and pending[0].arrival>time:time=pending[0].arrival
  while pending and pending[0].arrival<=time:ready.append(pending.pop(0))
  if not ready:continue
  j=ready.popleft();duration=min(quantum,remaining[j.name]);start=time;time+=duration;remaining[j.name]-=duration;timeline.append(Slice(j.name,start,time))
  while pending and pending[0].arrival<=time:ready.append(pending.pop(0))
  if remaining[j.name]>0:ready.append(j)
 return timeline,metrics(jobs,timeline)
