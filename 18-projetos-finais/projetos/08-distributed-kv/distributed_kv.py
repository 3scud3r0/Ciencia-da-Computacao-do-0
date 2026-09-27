from __future__ import annotations
from dataclasses import dataclass,field
import hashlib

@dataclass
class Node:
    name:str
    alive:bool=True
    data:dict[str,tuple[int,str]]=field(default_factory=dict)
    def put(self,key,version,value):
        if not self.alive:raise ConnectionError(self.name)
        current=self.data.get(key)
        if current is None or version>=current[0]:self.data[key]=(version,value)
    def get(self,key):
        if not self.alive:raise ConnectionError(self.name)
        return self.data.get(key)

class Cluster:
    def __init__(self,nodes,replication=3,write_quorum=2,read_quorum=2):
        if not nodes:raise ValueError("nodes")
        self.nodes=list(nodes);self.replication=min(replication,len(nodes));self.wq=write_quorum;self.rq=read_quorum;self.version=0
    def _owners(self,key):
        h=int.from_bytes(hashlib.sha256(key.encode()).digest()[:8],"big")
        start=h%len(self.nodes)
        return [self.nodes[(start+i)%len(self.nodes)] for i in range(self.replication)]
    def put(self,key,value):
        self.version+=1;version=self.version;acks=0
        for node in self._owners(key):
            try:node.put(key,version,value);acks+=1
            except ConnectionError:pass
        if acks<self.wq:raise RuntimeError("write quorum indisponível")
        return version
    def get(self,key):
        replies=[]
        for node in self._owners(key):
            try:
                value=node.get(key)
                if value is not None:replies.append((node,value))
            except ConnectionError:pass
        if len(replies)<self.rq:raise RuntimeError("read quorum indisponível")
        version,value=max((pair for _,pair in replies),key=lambda x:x[0])
        for node,pair in replies:
            if pair[0]<version:
                node.put(key,version,value)
        return value
    def set_alive(self,name,alive):
        for node in self.nodes:
            if node.name==name:node.alive=alive;return
        raise KeyError(name)
