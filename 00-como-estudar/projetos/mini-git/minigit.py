import hashlib,json,zlib
from pathlib import Path
class MiniGit:
 def __init__(self,root):
  self.root=Path(root);self.git=self.root/'.minigit';self.objects=self.git/'objects';self.refs=self.git/'refs'/'heads'
 def init(self):
  self.objects.mkdir(parents=True,exist_ok=True);self.refs.mkdir(parents=True,exist_ok=True)
  (self.git/'HEAD').write_text('ref: refs/heads/main\n',encoding='utf-8')
 def _write_object(self,kind,payload):
  raw=f'{kind} {len(payload)}\0'.encode()+payload;oid=hashlib.sha1(raw).hexdigest();path=self.objects/oid[:2]/oid[2:]
  path.parent.mkdir(parents=True,exist_ok=True)
  if not path.exists():path.write_bytes(zlib.compress(raw))
  return oid
 def read_object(self,oid):
  raw=zlib.decompress((self.objects/oid[:2]/oid[2:]).read_bytes());header,payload=raw.split(b'\0',1);kind,size=header.decode().split(' ')
  if int(size)!=len(payload):raise ValueError('objeto corrompido')
  return kind,payload
 def hash_blob(self,data):return self._write_object('blob',data if isinstance(data,bytes) else str(data).encode())
 def write_tree(self,entries):
  payload=json.dumps(dict(sorted(entries.items())),separators=(',',':'),ensure_ascii=False).encode()
  return self._write_object('tree',payload)
 def commit(self,tree,message,parent=None):
  payload=json.dumps({'tree':tree,'parent':parent,'message':message},separators=(',',':'),ensure_ascii=False).encode()
  oid=self._write_object('commit',payload);(self.refs/'main').write_text(oid+'\n',encoding='ascii');return oid
 def head(self):
  path=self.refs/'main';return path.read_text(encoding='ascii').strip() if path.exists() else None
