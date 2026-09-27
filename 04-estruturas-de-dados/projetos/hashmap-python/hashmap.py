from dataclasses import dataclass
_EMPTY=object();_TOMBSTONE=object()
@dataclass(slots=True)
class _Entry:key:object;value:object
class HashMap:
 def __init__(self,initial_capacity=8):
  c=4
  while c<initial_capacity:c<<=1
  self._table=[_EMPTY]*c;self._size=0;self._used=0
 def __len__(self):return self._size
 def _index(self,key):return hash(key)&(len(self._table)-1)
 def _find(self,key,insert):
  i=self._index(key);tomb=None
  for _ in range(len(self._table)):
   slot=self._table[i]
   if slot is _EMPTY:return tomb if insert and tomb is not None else i
   if slot is _TOMBSTONE:
    if insert and tomb is None:tomb=i
   elif slot.key==key:return i
   i=(i+1)&(len(self._table)-1)
  if insert and tomb is not None:return tomb
  raise KeyError(key)
 def _resize(self,n):
  old=list(self.items());self._table=[_EMPTY]*n;self._size=self._used=0
  for k,v in old:self[k]=v
 def __setitem__(self,key,value):
  if (self._used+1)/len(self._table)>.70:self._resize(len(self._table)*2)
  i=self._find(key,True);slot=self._table[i]
  if slot is _EMPTY:self._used+=1;self._size+=1
  elif slot is _TOMBSTONE:self._size+=1
  self._table[i]=_Entry(key,value)
 def __getitem__(self,key):
  i=self._find(key,False);slot=self._table[i]
  if slot is _EMPTY or slot is _TOMBSTONE:raise KeyError(key)
  return slot.value
 def __delitem__(self,key):
  i=self._find(key,False);slot=self._table[i]
  if slot is _EMPTY or slot is _TOMBSTONE:raise KeyError(key)
  self._table[i]=_TOMBSTONE;self._size-=1
 def items(self):
  for slot in self._table:
   if slot is not _EMPTY and slot is not _TOMBSTONE:yield slot.key,slot.value
