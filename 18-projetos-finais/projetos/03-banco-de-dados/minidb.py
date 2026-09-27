from __future__ import annotations
import json
from bisect import bisect_left
from pathlib import Path

class MiniDB:
    def __init__(self,path):
        self.path=Path(path);self.rows={};self.keys=[];self._tx=None
        if self.path.exists():self._recover()
    def _recover(self):
        for line in self.path.read_text(encoding="utf-8").splitlines():
            record=json.loads(line)
            if record["op"]=="commit":
                for change in record["changes"]:self._apply(change)
    def _apply(self,change):
        op=change["op"];key=change["key"]
        if op=="put":
            if key not in self.rows:self.keys.insert(bisect_left(self.keys,key),key)
            self.rows[key]=change["value"]
        elif op=="delete":
            if key in self.rows:
                del self.rows[key];self.keys.pop(bisect_left(self.keys,key))
    def begin(self):
        if self._tx is not None:raise RuntimeError("transação ativa")
        self._tx=[]
    def put(self,key,value):
        change={"op":"put","key":key,"value":value}
        if self._tx is None:self._commit([change])
        else:self._tx.append(change)
    def delete(self,key):
        change={"op":"delete","key":key}
        if self._tx is None:self._commit([change])
        else:self._tx.append(change)
    def commit(self):
        if self._tx is None:raise RuntimeError("sem transação")
        changes=self._tx;self._tx=None;self._commit(changes)
    def rollback(self):
        if self._tx is None:raise RuntimeError("sem transação")
        self._tx=None
    def _commit(self,changes):
        self.path.parent.mkdir(parents=True,exist_ok=True)
        record={"op":"commit","changes":changes}
        with self.path.open("a",encoding="utf-8") as f:
            f.write(json.dumps(record,ensure_ascii=False,separators=(",",":"))+"\n");f.flush()
        for change in changes:self._apply(change)
    def get(self,key):
        if key not in self.rows:raise KeyError(key)
        return self.rows[key]
    def scan(self,start=None,stop=None):
        lo=0 if start is None else bisect_left(self.keys,start)
        hi=len(self.keys) if stop is None else bisect_left(self.keys,stop)
        return [(k,self.rows[k]) for k in self.keys[lo:hi]]
