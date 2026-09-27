from http.server import BaseHTTPRequestHandler,ThreadingHTTPServer
from pathlib import Path
from urllib.parse import unquote
import json,threading
class WALStore:
    def __init__(self,path):
        self.path=Path(path);self.data={};self.lock=threading.Lock()
        if self.path.exists():
            for line in self.path.read_text(encoding="utf-8").splitlines():
                record=json.loads(line);self.data[record["key"]]=record["value"]
    def put(self,key,value):
        record={"key":key,"value":value};line=json.dumps(record,ensure_ascii=False,separators=(",",":"))
        with self.lock:
            self.path.parent.mkdir(parents=True,exist_ok=True)
            with self.path.open("a",encoding="utf-8") as f:f.write(line+"\n");f.flush()
            self.data[key]=value
    def get(self,key):
        with self.lock:return self.data.get(key)
class App:
    def __init__(self,store):self.store=store
    def get(self,path):
        if not path.startswith("/kv/"):return 404,{"error":"not found"}
        key=unquote(path[4:]);value=self.store.get(key)
        return (404,{"error":"missing"}) if value is None else (200,{"key":key,"value":value})
    def put(self,path,body):
        if not path.startswith("/kv/"):return 404,{"error":"not found"}
        key=unquote(path[4:]);payload=json.loads(body);self.store.put(key,payload["value"]);return 200,{"ok":True}
def handler_factory(app):
    class Handler(BaseHTTPRequestHandler):
        def _send(self,status,payload):
            body=json.dumps(payload,ensure_ascii=False).encode();self.send_response(status);self.send_header("Content-Type","application/json");self.send_header("Content-Length",str(len(body)));self.end_headers();self.wfile.write(body)
        def do_GET(self):self._send(*app.get(self.path))
        def do_PUT(self):
            length=int(self.headers.get("Content-Length","0"));self._send(*app.put(self.path,self.rfile.read(length).decode()))
        def log_message(self,*_):pass
    return Handler
def serve(path="data/store.wal",host="127.0.0.1",port=8088):
    app=App(WALStore(path));ThreadingHTTPServer((host,port),handler_factory(app)).serve_forever()
if __name__=="__main__":serve()
