from __future__ import annotations
import hashlib,json,zlib
from pathlib import Path

class GitLike:
    def __init__(self,root):
        self.root=Path(root);self.git=self.root/".gitlike";self.objects=self.git/"objects";self.refs=self.git/"refs"/"heads"
    def init(self):
        self.objects.mkdir(parents=True,exist_ok=True);self.refs.mkdir(parents=True,exist_ok=True)
        if not (self.git/"HEAD").exists():(self.git/"HEAD").write_text("ref: refs/heads/main\n",encoding="utf-8")
    def _write(self,kind,payload):
        raw=(kind+" "+str(len(payload))+"\0").encode()+payload
        oid=hashlib.sha1(raw).hexdigest();path=self.objects/oid[:2]/oid[2:];path.parent.mkdir(parents=True,exist_ok=True)
        if not path.exists():path.write_bytes(zlib.compress(raw))
        return oid
    def _read(self,oid):
        raw=zlib.decompress((self.objects/oid[:2]/oid[2:]).read_bytes());header,payload=raw.split(b"\0",1)
        kind,size=header.decode().split(" ")
        if int(size)!=len(payload):raise ValueError("objeto corrompido")
        return kind,payload
    def _head_ref(self):
        text=(self.git/"HEAD").read_text(encoding="utf-8").strip()
        return text[5:] if text.startswith("ref: ") else None
    def head(self):
        ref=self._head_ref()
        if ref:
            path=self.git/ref
            return path.read_text(encoding="ascii").strip() if path.exists() else None
        return (self.git/"HEAD").read_text(encoding="ascii").strip()
    def snapshot(self,files):
        tree={}
        for name,data in sorted(files.items()):
            if isinstance(data,str):data=data.encode()
            tree[name]=self._write("blob",data)
        return self._write("tree",json.dumps(tree,separators=(",",":"),sort_keys=True).encode())
    def commit(self,files,message):
        tree=self.snapshot(files);parent=self.head()
        payload=json.dumps({"tree":tree,"parent":parent,"message":message},separators=(",",":")).encode()
        oid=self._write("commit",payload);ref=self._head_ref()
        if ref is None:(self.git/"HEAD").write_text(oid+"\n",encoding="ascii")
        else:
            path=self.git/ref;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(oid+"\n",encoding="ascii")
        return oid
    def branch(self,name,start=None):
        if "/" in name or not name:raise ValueError("nome de branch inválido")
        oid=start or self.head()
        if oid is None:raise RuntimeError("sem commit")
        (self.refs/name).write_text(oid+"\n",encoding="ascii")
    def switch(self,name):
        path=self.refs/name
        if not path.exists():raise KeyError(name)
        (self.git/"HEAD").write_text("ref: refs/heads/"+name+"\n",encoding="utf-8")
    def files_at(self,commit=None):
        commit=commit or self.head()
        if commit is None:return {}
        kind,payload=self._read(commit)
        if kind!="commit":raise ValueError("não é commit")
        tree_oid=json.loads(payload)["tree"];kind,payload=self._read(tree_oid)
        if kind!="tree":raise ValueError("tree inválida")
        tree=json.loads(payload);result={}
        for name,oid in tree.items():
            kind,data=self._read(oid)
            if kind!="blob":raise ValueError("blob inválido")
            result[name]=data
        return result
    def diff(self,a,b):
        left=self.files_at(a);right=self.files_at(b);names=set(left)|set(right)
        return {name:("added" if name not in left else "removed" if name not in right else "modified")
                for name in sorted(names) if left.get(name)!=right.get(name)}
