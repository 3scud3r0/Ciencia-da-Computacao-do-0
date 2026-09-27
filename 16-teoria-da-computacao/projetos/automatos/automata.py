from collections import defaultdict,deque
EPSILON=None
class NFA:
 def __init__(self,start,accepting,transitions):
  self.start=start;self.accepting=set(accepting);self.transitions=defaultdict(set);self.alphabet=set()
  for (state,symbol),targets in transitions.items():
   self.transitions[(state,symbol)].update(targets)
   if symbol is not EPSILON:self.alphabet.add(symbol)
 def epsilon_closure(self,states):
  closure=set(states);stack=list(states)
  while stack:
   state=stack.pop()
   for nxt in self.transitions[(state,EPSILON)]:
    if nxt not in closure:closure.add(nxt);stack.append(nxt)
  return frozenset(closure)
 def step(self,states,symbol):
  reached=set()
  for state in self.epsilon_closure(states):reached.update(self.transitions[(state,symbol)])
  return self.epsilon_closure(reached)
 def accepts(self,text):
  states=self.epsilon_closure({self.start})
  for symbol in text:states=self.step(states,symbol)
  return bool(states&self.accepting)
 def determinize(self):
  start=self.epsilon_closure({self.start});queue=deque([start]);seen={start};transitions={};accepting=set()
  while queue:
   states=queue.popleft()
   if states&self.accepting:accepting.add(states)
   for symbol in sorted(self.alphabet):
    target=self.step(states,symbol);transitions[(states,symbol)]=target
    if target not in seen:seen.add(target);queue.append(target)
  return DFA(start,accepting,transitions)
class DFA:
 def __init__(self,start,accepting,transitions):self.start=start;self.accepting=set(accepting);self.transitions=dict(transitions)
 def accepts(self,text):
  state=self.start
  for symbol in text:
   if (state,symbol) not in self.transitions:return False
   state=self.transitions[(state,symbol)]
  return state in self.accepting
