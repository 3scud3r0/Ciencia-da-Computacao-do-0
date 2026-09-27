from bisect import bisect_left,bisect_right
class Leaf:
 __slots__=('keys','values','next')
 def __init__(self):self.keys=[];self.values=[];self.next=None
class Internal:
 __slots__=('keys','children')
 def __init__(self):self.keys=[];self.children=[]
class BPlusTree:
 def __init__(self,max_keys=4):
  if max_keys<3:raise ValueError('max_keys >= 3')
  self.max_keys=max_keys;self.root=Leaf()
 def search(self,key):
  leaf=self._leaf(key);i=bisect_left(leaf.keys,key)
  if i<len(leaf.keys) and leaf.keys[i]==key:return leaf.values[i]
  raise KeyError(key)
 def insert(self,key,value):
  split=self._insert(self.root,key,value)
  if split is not None:
   separator,right=split;root=Internal();root.keys=[separator];root.children=[self.root,right];self.root=root
 def _insert(self,node,key,value):
  if isinstance(node,Leaf):
   i=bisect_left(node.keys,key)
   if i<len(node.keys) and node.keys[i]==key:node.values[i]=value;return None
   node.keys.insert(i,key);node.values.insert(i,value)
   if len(node.keys)<=self.max_keys:return None
   mid=(len(node.keys)+1)//2;right=Leaf();right.keys=node.keys[mid:];right.values=node.values[mid:]
   node.keys=node.keys[:mid];node.values=node.values[:mid];right.next=node.next;node.next=right
   return right.keys[0],right
  i=bisect_right(node.keys,key);split=self._insert(node.children[i],key,value)
  if split is None:return None
  separator,right=split;node.keys.insert(i,separator);node.children.insert(i+1,right)
  if len(node.keys)<=self.max_keys:return None
  mid=len(node.keys)//2;promoted=node.keys[mid];right_node=Internal();right_node.keys=node.keys[mid+1:];right_node.children=node.children[mid+1:]
  node.keys=node.keys[:mid];node.children=node.children[:mid+1];return promoted,right_node
 def _leaf(self,key):
  node=self.root
  while isinstance(node,Internal):node=node.children[bisect_right(node.keys,key)]
  return node
 def range(self,start=None,stop=None):
  if start is None:
   node=self.root
   while isinstance(node,Internal):node=node.children[0]
   i=0
  else:node=self._leaf(start);i=bisect_left(node.keys,start)
  while node is not None:
   while i<len(node.keys):
    key=node.keys[i]
    if stop is not None and key>=stop:return
    yield key,node.values[i];i+=1
   node=node.next;i=0
 def validate(self):
  depths=[]
  def walk(node,depth,low=None,high=None):
   if node.keys!=sorted(node.keys):raise AssertionError('ordem')
   if isinstance(node,Leaf):
    if len(node.keys)!=len(node.values):raise AssertionError('alinhamento')
    for k in node.keys:
     if low is not None and k<low:raise AssertionError('low')
     if high is not None and k>=high:raise AssertionError('high')
    depths.append(depth);return
   if len(node.children)!=len(node.keys)+1:raise AssertionError('children')
   bounds=[low,*node.keys,high]
   for i,child in enumerate(node.children):walk(child,depth+1,bounds[i],bounds[i+1])
  walk(self.root,0)
  if len(set(depths))>1:raise AssertionError('folhas em níveis diferentes')
  return True
