import bisect,hashlib
def _hash(value):return int.from_bytes(hashlib.sha256(value.encode()).digest()[:8],"big")
class ConsistentHashRing:
 def __init__(self,replicas=128):
  if replicas<=0:raise ValueError("replicas")
  self.replicas=replicas;self._positions=[];self._owners={};self._nodes=set()
 def add_node(self,node):
  if node in self._nodes:return
  self._nodes.add(node)
  for replica in range(self.replicas):
   position=_hash(f"{node}#{replica}")
   while position in self._owners:position=(position+1)&((1<<64)-1)
   bisect.insort(self._positions,position);self._owners[position]=node
 def remove_node(self,node):
  if node not in self._nodes:return
  self._nodes.remove(node);doomed=[p for p in self._positions if self._owners[p]==node];s=set(doomed)
  self._positions=[p for p in self._positions if p not in s]
  for p in doomed:del self._owners[p]
 def locate(self,key):
  if not self._positions:raise LookupError("anel sem nós")
  i=bisect.bisect_left(self._positions,_hash(key))
  if i==len(self._positions):i=0
  return self._owners[self._positions[i]]
