from __future__ import annotations
class RedisLike:
    def __init__(self,clock):
        self.clock=clock;self.data={};self.expiry={}
    def _purge(self,key):
        deadline=self.expiry.get(key)
        if deadline is not None and self.clock()>=deadline:
            self.data.pop(key,None);self.expiry.pop(key,None)
    def execute(self,*parts):
        if not parts:raise ValueError("comando vazio")
        cmd=str(parts[0]).upper()
        if cmd=="SET":
            if len(parts) not in {3,5}:raise ValueError("SET key value [EX seconds]")
            key,value=str(parts[1]),str(parts[2]);self.data[key]=value;self.expiry.pop(key,None)
            if len(parts)==5:
                if str(parts[3]).upper()!="EX":raise ValueError("esperado EX")
                self.expiry[key]=self.clock()+float(parts[4])
            return "OK"
        if cmd=="GET":
            key=str(parts[1]);self._purge(key);return self.data.get(key)
        if cmd=="DEL":
            key=str(parts[1]);self._purge(key);existed=key in self.data
            self.data.pop(key,None);self.expiry.pop(key,None);return int(existed)
        if cmd=="INCR":
            key=str(parts[1]);self._purge(key)
            value=int(self.data.get(key,"0"))+1;self.data[key]=str(value);return value
        if cmd=="TTL":
            key=str(parts[1]);self._purge(key)
            if key not in self.data:return -2
            if key not in self.expiry:return -1
            return max(0,int(self.expiry[key]-self.clock()))
        raise ValueError(f"comando desconhecido: {cmd}")

def encode_resp(value):
    if value is None:return b"$-1\r\n"
    if isinstance(value,int):return (":" + str(value) + "\r\n").encode()
    if isinstance(value,str):
        data=value.encode();return ("$" + str(len(data)) + "\r\n").encode()+data+b"\r\n"
    raise TypeError(type(value))
